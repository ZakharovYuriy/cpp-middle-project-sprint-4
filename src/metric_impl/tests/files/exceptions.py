def Try_Exceptions():
    try:
        x = 1 / 0
        assert x == NaN
    except ZeroDivisionError:
        print("Ошибка")
    finally:
        print("Завершено")
