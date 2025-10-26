def test_Match_case(x):
    match x:
        case 1:
            return "one"
        case 2:
            return "two"
        case _:
            return "many"