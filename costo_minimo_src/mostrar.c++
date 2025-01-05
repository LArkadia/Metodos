#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

// Incluimos la librería donde están definidas las clases Screen, Color, etc.
#include "../SDL_Visor/SDL_Visor.hpp"  // Ajusta a tu nombre real, p. ej. "screen.h" o algo similar

// Para simplificar referencias:
using std::vector;
using std::string;

//////////////////////////////////////////////////
// Funciones auxiliares para leer los archivos. //
//////////////////////////////////////////////////

/**
 * Lee el archivo "datos.txt" y llena la estructura de datos:
 *   - fila 0: oferta de cada almacén
 *   - fila 1: demanda de cada cliente
 *   - filas restantes: matriz de costos de envío (almacenes x clientes)
 */
bool leerDatos(const std::string& archivo_datos,
               std::vector<std::vector<int>>& datos)
{
    std::ifstream in(archivo_datos);
    if(!in.is_open())
    {
        std::cerr << "No se pudo abrir el archivo: " << archivo_datos << std::endl;
        return false;
    }

    std::string linea;
    while(std::getline(in, linea))
    {
        // Ignoramos comentarios o líneas vacías
        if(linea.empty() || linea.rfind("--", 0) == 0)
            continue;
        
        std::istringstream iss(linea);
        int valor;
        vector<int> fila;
        while(iss >> valor)
        {
            fila.push_back(valor);
        }

        if(!fila.empty())
            datos.push_back(fila);
    }
    in.close();
    return true;
}

/**
 * Lee el archivo "resultados.txt" y:
 *   - llena un vector temporal con las asignaciones (almacén, cliente, cantidad)
 *   - el último número leido lo interpreta como coste total
 */
bool leerResultados(const std::string& archivo_resultados,
                    std::vector<std::vector<int>>& asignaciones,
                    int& costo_total)
{
    std::ifstream in(archivo_resultados);
    if(!in.is_open())
    {
        std::cerr << "No se pudo abrir el archivo: " << archivo_resultados << std::endl;
        return false;
    }

    std::string linea;
    vector<vector<int>> temp;
    while(std::getline(in, linea))
    {
        if(linea.empty()) 
            continue;

        std::istringstream iss(linea);
        int valor;
        vector<int> fila;
        while(iss >> valor)
        {
            fila.push_back(valor);
        }

        if(!fila.empty())
            temp.push_back(fila);
    }
    in.close();

    // El último renglón debe ser el costo total (un único valor).
    // El resto se consideran asignaciones [almacen, cliente, cantidad].
    if(!temp.empty())
    {
        // El último vector de la lista debería tener 1 valor
        vector<int>& ultima_fila = temp.back();
        if(ultima_fila.size() == 1)
        {
            costo_total = ultima_fila[0];
            temp.pop_back();
        }
        else
        {
            std::cerr << "Error en resultados.txt: el último renglón debe ser el costo total.\n";
            return false;
        }
    }
    // Copiamos todo lo demás a "asignaciones"
    asignaciones = temp;

    return true;
}

//////////////////////////////////////////////////////////////
// Funciones para convertir datos a una estructura interna. //
//////////////////////////////////////////////////////////////

/**
 * A partir de los datos leídos de "datos.txt":
 *  - fila 0 = vector<int> con la oferta de cada almacén
 *  - fila 1 = vector<int> con la demanda de cada cliente
 *  - fila 2 en adelante = matriz de costos
 */
void procesarDatos(const std::vector<std::vector<int>>& datos,
                   vector<int>& oferta,
                   vector<int>& demanda,
                   vector<vector<int>>& costos)
{
    if(datos.size() < 3)
    {
        std::cerr << "El archivo de datos no tiene el formato esperado.\n";
        return;
    }

    // fila 0 -> oferta
    oferta = datos[0];
    // fila 1 -> demanda
    demanda = datos[1];

    // las filas restantes son la matriz de costos
    size_t w = oferta.size();   // número de almacenes
    size_t c = demanda.size();  // número de clientes
    costos.resize(w, vector<int>(c, 0));

    // A partir de la fila 2, tenemos w filas
    // datos[2], datos[3], ..., datos[2 + w - 1]
    // cada fila es de largo c
    for(size_t i = 0; i < w; i++)
    {
        for(size_t j = 0; j < c; j++)
        {
            costos[i][j] = datos[2 + i][j];
        }
    }
}

/**
 * A partir de los datos leídos de "resultados.txt":
 *   - asignaciones = (almacén, cliente, cantidad)
 *   - se construye una matriz shipments[w][c] con la cantidad enviada
 */
void procesarResultados(const std::vector<std::vector<int>>& asignaciones,
                        size_t w, size_t c,
                        vector<vector<int>>& shipments)
{
    shipments.resize(w, vector<int>(c, 0));
    for(const auto& fila : asignaciones)
    {
        if(fila.size() < 3) 
            continue; // ignorar malformados

        int almacen = fila[0];
        int cliente = fila[1];
        int cantidad = fila[2];

        // Ojo: si en el archivo los almacenes y clientes empiezan en 1,
        // ajustamos a índice base 0
        if(almacen >= 1 && almacen <= (int)w &&
           cliente >= 1 && cliente <= (int)c)
        {
            shipments[almacen - 1][cliente - 1] = cantidad;
        }
        else
        {
            std::cerr << "Asignación inválida en resultados.txt: " 
                      << almacen << ", " << cliente 
                      << ", " << cantidad << std::endl;
        }
    }
}

/////////////////////////////////////////////////////
// Función para dibujar la tabla con la librería.  //
/////////////////////////////////////////////////////

/**
 * Dibuja la tabla en la ventana. La tabla tendrá:
 *  - Filas = w + 1 (almacenes + 1 fila para la demanda)
 *  - Columnas = c + 1 (clientes + 1 columna para la oferta)
 *
 * Estructura conceptual (índices de la tabla):
 *       C0      C1     C2     ...      C(c)
 *  R0   ""    Clien1  Clien2  ...    ClienC
 *  R1   Alm1    cost     cost  ...      cost  |   oferta(Alm1)
 *  R2   Alm2    cost     cost  ...      cost  |   oferta(Alm2)
 *  ...
 *  Rw   AlmW    cost     cost  ...      cost  |   oferta(AlmW)
 * 
 *  (demanda) en la última fila
 * 
 *  En cost se mostrará:
 *     - solo el costo de envío si shipments[i][j] == 0
 *     - costo + "(" + cantidad + ")" si shipments[i][j] > 0
 */
void dibujarTabla(
    vsr::Screen& screen,
    const vector<int>& oferta,
    const vector<int>& demanda,
    const vector<vector<int>>& costos,
    const vector<vector<int>>& shipments,
    int costo_envios
)
{
    // Cantidades
    size_t w = oferta.size();   // n almacenes
    size_t c = demanda.size();  // n clientes

    // Dimensiones visuales de cada celda (ajusta a tu gusto).
    const int cellWidth  = 100;
    const int cellHeight = 40;

    // Posición inicial de la tabla en la ventana
    const int startX = 50;
    const int startY = 50;

    // Colores básicos
    vsr::Color colorLineas(0,0,0);         // negro para líneas
    vsr::Color colorTexto(0,0,0);         // texto en negro
    vsr::Color colorResaltado(255,255,0); // amarillo (si quieres destacar algo)

    // Fuente default (asume que tu Screen ya tiene una fuente cargada o usa la default)
    // Si no, llama a: screen.Load_font("default", "ruta.ttf", 16); etc.

    //////////////////////////////////////////////////
    // DIBUJAR LÍNEAS DE LA TABLA (GRILLA)
    //////////////////////////////////////////////////
    // Filas totales = w + 1 (almacenes) + 1 (cabecera) + 1 (fila de demandas) = w + 2
    // Columnas totales = c + 1 (clientes) + 1 (columna de ofertas) = c + 2
    int rowCount = (int)w + 2; // +1 para encabezado, +1 para fila de demanda
    int colCount = (int)c + 2; // +1 para encabezado, +1 para columna de oferta

    // Dibujar líneas horizontales
    for(int r = 0; r <= rowCount; r++)
    {
        int y = startY + r * cellHeight;
        screen.Draw_line_pos(startX, y, startX + colCount * cellWidth, y, colorLineas);
    }
    // Dibujar líneas verticales
    for(int col = 0; col <= colCount; col++)
    {
        int x = startX + col * cellWidth;
        screen.Draw_line_pos(x, startY, x, startY + rowCount * cellHeight, colorLineas);
    }

    //////////////////////////////////////////////////
    // ESCRIBIR CONTENIDO DE CADA CELDA
    //////////////////////////////////////////////////
    //  R0, C0 (vacío), R0, C1..C(c) (nombres de clientes),
    //  R1..R(w), C0 (nombres de almacenes),
    //  R(w+1), C1..C(c) (demandas),
    //  R1..R(w), C(c+1) (ofertas)

    // (A) R0, C1..C(c): nombres de clientes
    for(size_t j = 0; j < c; j++)
    {
        int cellX = startX + (j+1)*cellWidth; 
        int cellY = startY; // cabecera
        // Texto "Cliente j+1" o lo que desees
        std::ostringstream oss;
        oss << "Cliente " << (j+1);
        screen.Show_text(cellX + 5, cellY + 5, String(oss.str()), colorTexto);
    }

    // (B) R1..R(w), C0: nombres de almacenes
    for(size_t i = 0; i < w; i++)
    {
        int cellX = startX; 
        int cellY = startY + (i+1)*cellHeight;
        std::ostringstream oss;
        oss << "Alm " << (i+1);
        screen.Show_text(cellX + 5, cellY + 5, oss.str(), colorTexto);
    }

    // (C) Celdas de costos (R1..R(w), C1..C(c))
    for(size_t i = 0; i < w; i++)
    {
        for(size_t j = 0; j < c; j++)
        {
            int cellX = startX + (j+1)*cellWidth;
            int cellY = startY + (i+1)*cellHeight;

            int cost = costos[i][j];
            int sent = shipments[i][j];

            // Si no hay venta, solo mostramos el costo
            // Si hay venta, mostramos "cost(cantidad)"
            std::ostringstream oss;
            if(sent == 0)
            {
                oss << cost;
            }
            else
            {
                oss << cost << "(" << sent << ")";
            }
            screen.Show_text(cellX + 5, cellY + 5, oss.str(), colorTexto);
        }
    }

    // (D) Última fila (R = w+1) con las demandas
    {
        int rowY = startY + (w+1)*cellHeight;
        for(size_t j = 0; j < c; j++)
        {
            int cellX = startX + (j+1)*cellWidth;
            std::ostringstream oss;
            oss << demanda[j];
            screen.Show_text(cellX + 5, rowY + 5, oss.str(), colorTexto);
        }
        // Texto "Demanda" en C0
        {
            int cellX = startX;
            screen.Show_text(cellX + 5, rowY + 5, "Demanda", colorTexto);
        }
    }

    // (E) Última columna (C = c+1) con las ofertas de cada almacén
    {
        for(size_t i = 0; i < w; i++)
        {
            int cellX = startX + (c+1)*cellWidth;
            int cellY = startY + (i+1)*cellHeight;

            std::ostringstream oss;
            oss << oferta[i];
            screen.Show_text(cellX + 5, cellY + 5, oss.str(), colorTexto);
        }
        // Texto "Oferta" en R0
        {
            int cellX = startX + (c+1)*cellWidth;
            int cellY = startY;
            screen.Show_text(cellX + 5, cellY + 5, "Oferta", colorTexto);
        }
    }

    //////////////////////////////////////////////////
    // TEXTO FINAL: "Coste de los envíos: ..."
    //////////////////////////////////////////////////
    {
        int finalTextX = startX;
        int finalTextY = startY + (rowCount*cellHeight) + 30; 
        std::ostringstream oss;
        oss << "Coste de los envios: $" << costo_envios;
        screen.Show_text(finalTextX, finalTextY, oss.str(), colorTexto);
    }
}

/////////////////////////////////////////////////////
//                       MAIN                      //
/////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    // 1) Leer y procesar el archivo "datos"
    std::vector<std::vector<int>> datos;
    if(!leerDatos("datos", datos))
    {
        return 1;
    }

    // 2) Leer y procesar el archivo "resultados"
    std::vector<std::vector<int>> asignaciones;
    int costo_envios = 0;
    if(!leerResultados("resultados", asignaciones, costo_envios))
    {
        return 1;
    }

    // 3) Extraer de "datos" la oferta, la demanda y la matriz de costos
    vector<int> oferta;
    vector<int> demanda;
    vector<vector<int>> costos; // [almacen][cliente]
    procesarDatos(datos, oferta, demanda, costos);

    // 4) A partir de las asignaciones, construir la matriz "shipments"
    //    con la cantidad enviada por [almacen][cliente]
    size_t w = oferta.size();
    size_t c = demanda.size();
    vector<vector<int>> shipments;
    procesarResultados(asignaciones, w, c, shipments);

    // -------------------------------------------------------------
    // A continuación, inicializamos SDL y creamos nuestra ventana.
    // Usamos la clase vsr::Screen. Ajusta ancho/alto a tu preferencia.
    // -------------------------------------------------------------
    const uint16_t windowWidth  = 1000;
    const uint16_t windowHeight = 600;

    // Creamos la pantalla
    vsr::Screen screen("metodo del costo minimo", windowWidth, windowHeight, SDL_RENDERER_ACCELERATED);

    // Inicializamos la parte de imágenes y fuentes (si corresponde)
    // (Asegúrate de tener SDL2_ttf y SDL2_image instalados).
    // Si tu clase Screen exige llamadas específicas, ajústalo.
    screen.Init_IMG();
    screen.Init_TTF("default", "../SDL_Visor/fonts/NotoSans/NotoSans-Bold.ttf", 18); 
    screen.Set_default_font("default");

    // Event handler vacío (por si no queremos hacer nada especial con los eventos).
    auto eventHandler = [](SDL_Event& e){
        // Aquí podrías manejar clicks, etc.  
    };
    screen.Set_events_handler(eventHandler);

    // Bucle principal
    bool running = true;
    while(running)
    {
        // Manejo de eventos (si el usuario cierra la ventana, etc.)
        if(!screen.Handle_events())
        {
            running = false;
        }

        // Limpiamos la pantalla con color blanco, por ejemplo
        vsr::Color blanco(255,255,255);
        screen.Clean_screen(blanco);

        // Dibujamos la tabla
        dibujarTabla(screen, oferta, demanda, costos, shipments, costo_envios);

        // Mostramos lo que se dibujó
        screen.Present_renderer();
    }

    return 0;
}
