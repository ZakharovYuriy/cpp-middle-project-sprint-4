def __test_multiparameters__(a, b, c=5, *args, **kwargs):
    assert a + b == c