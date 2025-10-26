def TestLoops(n):
    for i in range(n):
        while (i < n):
            if i % 2 == 0:
                print(i)
            i += 1
    return True