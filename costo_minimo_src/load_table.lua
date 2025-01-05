#!/usr/bin/lua5.4
local archivo_datos = arg[1]

local function separar_numeros(str) -- Funcion para separar los números por espacios
    local datos = {}
    for numero in string.gmatch(str,"%S+") do
        table.insert(datos,tonumber(numero))
    end
    return datos
end

local almacenes, clientes, costos_tabla = {}, {}, {} -- Constructores de tablas

local estados = { -- pseudo funcion switch
    function (datos) almacenes = separar_numeros(datos) end,
    function (datos) clientes = separar_numeros(datos) end,
    function (datos) table.insert(costos_tabla,separar_numeros(datos)) end
}

local estado = 1

for line in io.lines(archivo_datos) do -- Cargar lineas del archivo de datos
    if string.sub(line, 1, 2) ~= "--" then
        estados[estado](line)
        if estado < 3 then
            estado = estado + 1
        end
    end
end

local costos = {}

for almacen, clientes_posibles in pairs(costos_tabla) do -- Creación de la tabla de costos de envío
    for cliente, costo in pairs(clientes_posibles) do
        table.insert(costos, {almacen, cliente, costo})
    end
end

-- Ordenar la tabla 'costos' del envio mas barato al más caro
table.sort(costos, function(a, b) return a[3] < b[3] end)

local ventas = {}
local costo_envios = 0

for _, costo in ipairs(costos) do
    local almacen = costo[1]
    local cliente = costo[2]
    local venta = math.min(almacenes[almacen],clientes[cliente])
    table.insert(ventas,{almacen,cliente,venta})
    almacenes[almacen] = almacenes[almacen] - venta
    clientes[cliente] = clientes[cliente] - venta
    costo_envios = costo_envios + venta*costo[3]
end

local resultados = io.output("resultados")

for _, venta in ipairs(ventas) do
    if venta[3] ~= 0 then
        io.write(venta[1]," ",venta[2]," ",venta[3],"\n")
    end
end
io.write(costo_envios)

io.close(resultados)

os.execute("./mostrar.out")


