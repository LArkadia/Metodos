#include "../SDL_Visor/SDL_Visor.hpp"
#include "parser.cpp"
#include <map>
#include <cmath>

std::map<String, vsr::Color*> Init_colors();
std::vector<PSR::Ecuation*> Obtener_sistema();
struct GraphParameters {
    int grid_x_start;
    int grid_y_start;
    int grid_width;
    int grid_height;
    float pixels_per_unit_x;
    float pixels_per_unit_y;
    float x_units;
    float y_units;
};
GraphParameters Crear_grafica_fondo(vsr::Screen* window, std::vector<PSR::Ecuation*> sistema);
void Graficar_ecuacion(vsr::Screen* window, PSR::Ecuation* ecuacion, vsr::Color* color, GraphParameters& params);
void Achurar_region_no_factible(vsr::Screen* window, PSR::Ecuation* ecuacion, vsr::Color* color, GraphParameters& params);

auto colores = Init_colors();

int main() {
    auto sistema = Obtener_sistema();

    vsr::Screen window("Método gráfico de solución", 900, 800, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    window.Init_TTF("NotoSans", "../SDL_Visor/fonts/NotoSans/NotoSans-Thin.ttf", 15);

    auto graph_params = Crear_grafica_fondo(&window, sistema);

    while (window.Handle_events()) {
        window.Draw_saved_texture("bg", nullptr);

        // Achurar la región no factible para cada restricción
        for (size_t i = 1; i < sistema.size(); ++i) {
            Achurar_region_no_factible(&window, sistema[i],colores["rojo"], graph_params);
        }

        // Graficar las ecuaciones del sistema, excluyendo la ecuación objetivo en posición 0
        for (size_t i = 1; i < sistema.size(); ++i) {
            // Usar colores diferentes para cada ecuación si lo deseas
            Graficar_ecuacion(&window, sistema[i], colores["azul"], graph_params);
        }

        window.Present_renderer();
        SDL_Delay(100);
    }

    return 0;
}

std::map<String, vsr::Color*> Init_colors() {
    return std::map<String, vsr::Color*>({
        {"blanco", new vsr::Color(255, 255, 255, 255)},
        {"negro", new vsr::Color(0, 0, 0, 255)},
        {"gris", new vsr::Color(200, 200, 200, 100)}, // Transparente para el achurado
        {"rojo", new vsr::Color(255, 0, 0, 255)},
        {"verde", new vsr::Color(0, 255, 0, 255)},
        {"azul", new vsr::Color(0, 0, 255, 255)},
        {"aqua", new vsr::Color(0, 100, 100, 255)},
        {"cyan", new vsr::Color(0, 255, 255, 255)}
    });
}

std::vector<PSR::Ecuation*> Obtener_sistema() {
    std::vector<PSR::Ecuation*> Ecuaciones;
    float x1, x2, result;
    int cant_ecuaciones;
    char e;
    std::cout << "Ingrese la ecuación a maximizar o minimizar en el formato \"{x1} {x2}\"\n";
    scanf("%f %f", &x1, &x2);
    std::cout << "Ingrese la cantidad de restricciones\n";
    scanf("%d", &cant_ecuaciones);
    Ecuaciones.push_back(new PSR::Ecuation(x1, x2, 0, '=')); // Ecuación objetivo
    for (int i = 0; i < cant_ecuaciones; i++) {
        std::cout << "Ingrese la restricción en el formato \"{x1} {x2} {operador} {resultado}\"\n";
        scanf("%f %f %c %f", &x1, &x2, &e, &result);
        Ecuaciones.push_back(new PSR::Ecuation(x1, x2, result, e));
    }

    return Ecuaciones;
}

GraphParameters Crear_grafica_fondo(vsr::Screen* window, std::vector<PSR::Ecuation*> sistema) {
    int width, height;
    window->Get_window_sizes(&width, &height);
    window->Create_texture("bg", width, height);

    // Definir el área de la cuadrícula
    int grid_x_start = 40;
    int grid_y_start = 0;
    int grid_width = width - grid_x_start;
    int grid_height = height - 20;

    // Analizar las ecuaciones para encontrar los valores máximos en X y Y
    float max_x = 0.0f;
    float max_y = 0.0f;

    for (size_t i = 1; i < sistema.size(); ++i) {
        PSR::Ecuation* ecuacion = sistema[i];
        float a = ecuacion->get_x1();
        float b = ecuacion->get_x2();
        float c = ecuacion->get_result();

        // Encontrar intersecciones con los ejes
        float x_intercept = (a != 0) ? c / a : 0.0f;
        float y_intercept = (b != 0) ? c / b : 0.0f;

        // Considerar solo valores positivos
        if (x_intercept > max_x) max_x = x_intercept;
        if (y_intercept > max_y) max_y = y_intercept;
    }

    // Asegurarse de que max_x y max_y sean al menos 1 para evitar divisiones por cero
    if (max_x < 1.0f) max_x = 1.0f;
    if (max_y < 1.0f) max_y = 1.0f;

    // Añadir un margen del 10%
    max_x *= 1.1f;
    max_y *= 1.1f;

    // Dividir los ejes en 50 divisiones
    int divisions = 20;

    // Calcular unidades por división
    float units_per_div_x = max_x / divisions;
    float units_per_div_y = max_y / divisions;

    // Calcular píxeles por unidad
    float pixels_per_unit_x = static_cast<float>(grid_width) / max_x;
    float pixels_per_unit_y = static_cast<float>(grid_height) / max_y;

    // Limpiar la pantalla y dibujar los ejes
    window->Clean_screen(*colores["blanco"]);

    // Eje X
    window->Draw_line_pos(
        grid_x_start, grid_y_start + grid_height,
        grid_x_start + grid_width, grid_y_start + grid_height,
        *colores["negro"]);

    // Eje Y
    window->Draw_line_pos(
        grid_x_start, grid_y_start,
        grid_x_start, grid_y_start + grid_height,
        *colores["negro"]);

    // Dibujar líneas de la cuadrícula y etiquetas
    // Líneas verticales y etiquetas en X
    for (int i = 0; i <= divisions; i++) {
        int x_pos = grid_x_start + static_cast<int>(i * units_per_div_x * pixels_per_unit_x);
        window->Draw_line_pos(
            x_pos, grid_y_start,
            x_pos, grid_y_start + grid_height,
            *colores["gris"]);

        // Etiqueta X
        std::string label = std::to_string(i * units_per_div_x);
        window->Show_text(
            x_pos - 10,
            grid_y_start + grid_height - 2,
            label.substr(0, label.find('.') + 3), // Mostrar dos decimales
            *colores["negro"]);
    }

    // Líneas horizontales y etiquetas en Y
    for (int i = 0; i <= divisions; i++) {
        int y_pos = grid_y_start + grid_height - static_cast<int>(i * units_per_div_y * pixels_per_unit_y);
        window->Draw_line_pos(
            grid_x_start,
            y_pos,
            grid_x_start + grid_width,
            y_pos,
            *colores["gris"]);

        // Etiqueta Y
        std::string label = std::to_string(i * units_per_div_y);
        window->Show_text(
            grid_x_start - 35,
            y_pos - 5,
            label.substr(0, label.find('.') + 3), // Mostrar dos decimales
            *colores["negro"]);
    }

    window->End_texture();

    // Devolver los parámetros de la gráfica
    GraphParameters params = {
        grid_x_start,
        grid_y_start,
        grid_width,
        grid_height,
        pixels_per_unit_x,
        pixels_per_unit_y,
        max_x,
        max_y
    };

    return params;
}

void Graficar_ecuacion(vsr::Screen* window, PSR::Ecuation* ecuacion, vsr::Color* color, GraphParameters& params) {
    // Obtener los coeficientes de la ecuación
    float a = ecuacion->get_x1();
    float b = ecuacion->get_x2();
    float c = ecuacion->get_result();

    // Encontrar dos puntos para dibujar la línea
    float x1 = 0, y1 = 0;
    float x2 = 0, y2 = 0;

    if (b != 0) {
        // Cuando x1 = 0
        x1 = 0;
        y1 = c / b;

        // Cuando x1 = max_x
        x2 = params.x_units;
        y2 = (c - a * x2) / b;
    } else if (a != 0) {
        // Cuando x2 = 0
        y1 = 0;
        x1 = c / a;

        // Cuando x2 = max_y
        y2 = params.y_units;
        x2 = (c - b * y2) / a;
    } else {
        // Caso especial: a y b son cero, no se puede graficar
        return;
    }

    // Asegurarse de que los puntos estén dentro del rango
    if (y1 < 0 || y1 > params.y_units) {
        y1 = std::max(0.0f, std::min(y1, params.y_units));
    }
    if (y2 < 0 || y2 > params.y_units) {
        y2 = std::max(0.0f, std::min(y2, params.y_units));
    }
    if (x1 < 0 || x1 > params.x_units) {
        x1 = std::max(0.0f, std::min(x1, params.x_units));
    }
    if (x2 < 0 || x2 > params.x_units) {
        x2 = std::max(0.0f, std::min(x2, params.x_units));
    }

    // Convertir a coordenadas de píxeles
    int x_pixel_1 = params.grid_x_start + static_cast<int>(x1 * params.pixels_per_unit_x);
    int y_pixel_1 = params.grid_y_start + params.grid_height - static_cast<int>(y1 * params.pixels_per_unit_y);

    int x_pixel_2 = params.grid_x_start + static_cast<int>(x2 * params.pixels_per_unit_x);
    int y_pixel_2 = params.grid_y_start + params.grid_height - static_cast<int>(y2 * params.pixels_per_unit_y);

    // Dibujar la línea de la ecuación
    window->Draw_line_pos(
        x_pixel_1, y_pixel_1,
        x_pixel_2, y_pixel_2,
        *color);
}

void Achurar_region_no_factible(vsr::Screen* window, PSR::Ecuation* ecuacion, vsr::Color* color, GraphParameters& params) {
    // Obtener los coeficientes de la ecuación
    float a = ecuacion->get_x1();
    float b = ecuacion->get_x2();
    float c = ecuacion->get_result();
    char operador = ecuacion->get_operator(); // Necesitamos esta función en la clase PSR::Ecuation

    // Definir la función que verifica si un punto está en la región no factible
    auto es_no_factible = [=](float x, float y) -> bool {
        float valor = a * x + b * y;
        switch (operador) {
            case '<':
                return valor > c; // Región no factible es donde la restricción no se cumple
            case '>':
                return valor < c;
            case '=':
                return valor != c;
            default:
                return false;
        }
    };

    // Dibujar líneas de achurado sobre la región no factible
    int step = 10; // Espacio entre líneas de achurado
    for (int y_pixel = params.grid_y_start; y_pixel <= params.grid_y_start + params.grid_height; y_pixel += step) {
        // Convertir y_pixel a coordenada y
        float y = params.y_units - ((float)(y_pixel - params.grid_y_start) / params.pixels_per_unit_y);
        for (int x_pixel = params.grid_x_start; x_pixel <= params.grid_x_start + params.grid_width; x_pixel += step) {
            // Convertir x_pixel a coordenada x
            float x = (float)(x_pixel - params.grid_x_start) / params.pixels_per_unit_x;

            if (es_no_factible(x, y)) {
                // Dibujar un pequeño rectángulo o punto en (x_pixel, y_pixel)
                window->Draw_rectangle(x_pixel, y_pixel, 5, 5, *color);
            }
        }
    }
}
/*
void Achurar_region_no_factible(vsr::Screen* window, PSR::Ecuation* ecuacion, vsr::Color* color, GraphParameters& params) {
    // Obtener los coeficientes de la ecuación
    float a = ecuacion->get_x1();
    float b = ecuacion->get_x2();
    float c = ecuacion->get_result();
    char operador = ecuacion->get_operator();

    // Definir la función que verifica si un punto está en la región no factible
    auto es_no_factible = [=](float x, float y) -> bool {
        float valor = a * x + b * y;
        switch (operador) {
            case '<':
            case 'L':
                return valor > c;
            case '>':
            case 'G':
                return valor < c;
            case '=':
                return valor != c;
            default:
                return false;
        }
    };

    // Dibujar líneas de achurado diagonales sobre la región no factible
    int step = 10; // Espacio entre líneas de achurado
    for (int x_pixel = params.grid_x_start; x_pixel <= params.grid_x_start + params.grid_width; x_pixel += step) {
        int y_start = params.grid_y_start;
        int y_end = params.grid_y_start + params.grid_height;

        // Dibujar línea diagonal desde (x_pixel, y_start) hasta (x_pixel + offset, y_end)
        int x_current = x_pixel;
        int y_current = y_start;

        while (x_current <= params.grid_x_start + params.grid_width && y_current <= y_end) {
            // Convertir (x_current, y_current) a coordenadas (x, y)
            float x = (float)(x_current - params.grid_x_start) / params.pixels_per_unit_x;
            float y = params.y_units - ((float)(y_current - params.grid_y_start) / params.pixels_per_unit_y);

            if (es_no_factible(x, y)) {
                // Dibujar punto o pequeño segmento
                window->Draw_point(x_current, y_current, *color);
            }

            x_current += 1;
            y_current += 1;
        }
    }
}
*/