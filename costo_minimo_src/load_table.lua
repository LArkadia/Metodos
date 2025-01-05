#!/usr/bin/lua
local archivo_datos = arg[1]

local function separar_numeros(str)
    local datos = {}
    for numero in string.gmatch(str,"%S+") do
        table.insert(datos,tonumber(numero))
    end
    return datos
end

local provedores, clientes, costos = {}, {}, {}

local estados = {
    function (datos)
        provedores = separar_numeros(datos)
    end,

    function (datos)
        clientes = separar_numeros(datos)
    end,

    function (datos)
        table.insert(costos,separar_numeros(datos))
    end
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

print("provedores:")
for _,provedor in pairs(costos) do
    print(provedor)
end