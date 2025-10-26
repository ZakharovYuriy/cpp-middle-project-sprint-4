def Testnestedif(x, y):
    if x > 0:
        if y > 0:
            assert x == y
        elif x < 0:
            return 0
        else:
            return 1
    return -1
