-- time lua vm0.lua
-- time luajit vm0.lua
local a = 10
local b = 2
local c = 200000000
local d = 7
local e = 1
local f = 0
local i = a

while i < c do
    if i % d == 0 then
        while i < c do
            i = i + e

            if i % d == f then
                break
            end
        end
    else
        i = i + b
    end
end

print('i:', i)