# time python2 vm0.py
# time python3 vm0.py
# time pypy vm0.py
# time pypy3 vm0.py
a = 10
b = 2
c = 200000000
d = 7
e = 1
i = a

while i < c:
    if i % d == 0:
        while i < c:
            i = i + e
    else:
        i = i + b

print('i:', i)