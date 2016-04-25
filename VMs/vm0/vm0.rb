# time ruby vm0.rb
a = 10
b = 2
c = 200000000
d = 7
e = 1
f = 0
i = a

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

puts "i: #{i}"