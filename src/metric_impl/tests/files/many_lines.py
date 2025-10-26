def testmultiline():
    data = [
        1, 2, 3,
        4, 5, 6
    ]
    total = sum(data)
    
    print("Больше 10")
    print("Меньше или равно 10")
        
    call_unexisting_function1(total)
    call_unexisting_function2(total)
    call_unexisting_function3(total)

    assert total == 6