# -------------------------------
# Максимально "разреженный" пример
# -------------------------------


# 1. Пустые строки в начале файла допустимы


def example_function(

    x,

    y = 42

):

    

    # Пустая строка после комментария
    

    if x > y:
        
        
        # Пустая строка внутри блока if
        
        print(
            
            
            "x is greater"
            
            
        )
        
        
    else:
        
        
        # Ещё одна пустая строка
        
        print(
            
            "x is smaller or equal"
            
        )
        

    # Между выражениями тоже можно вставить пустые строки
    

    result = (
        
        
        x
        +
        y
        
        
    )
    

    
    return result
    


# 2. Пустые строки между определениями


class MyClass:
    
    
    """Докстринг с пустыми строками
    
    
    и ещё одной пустой строкой
    """
    
    
    def __init__(

        self,

        value = None

    ):
        
        
        # Внутри конструктора
        
        
        self.value = value
        

    
    def do_something(

        self

    ):
        
        
        # Пустая строка перед pass
        

        pass
        
        

# 3. Пустые строки в списках, словарях, кортежах и вызовах

values = [
    
    
    1,
    
    
    2,
    
    
    3,
    
    
]


mapping = {
    
    
    "a":
    
    
    1,
    
    
    "b":
    
    
    2,
    
    
}


result = sum(
    
    
    values
    
    
)


# 4. Пустые строки внутри цикла и условий

for i in values:
    
    
    if i % 2 == 0:
        
        
        continue
        
        
    else:
        
        
        # комментарий между пустыми строками
        
        
        print(i)
        
        
# 5. Пустые строки после return, pass, break — допустимы

def another_func():
    
    
    pass
    
    
    

# 6. Лямбды и выражения — можно разделять пустыми строками внутри скобок

calc = (
    
    
    lambda x,
           y:
           
           
           x
           +
           y
    
    
)


# 7. Пустые строки в try/except/else/finally

try:
    
    
    risky_operation()
    
    
except Exception as e:
    
    
    # обработка ошибки
    
    
    print(e)
    
    
else:
    
    
    # ветка без исключения
    
    
    pass
    
    
finally:
    
    
    # завершающий блок
    
    
    print("Done")
    


# 8. Пустые строки в декораторах

@staticmethod


def decorated_function():
    
    
    pass
    


# 9. Пустые строки в списковых включениях

squares = [
    
    
    x ** 2
    
    
    for x in range(
        
        
        10
        
        
    )
    
    
    if x % 2 == 0
    
    
]


# 10. Пустые строки в конце файла допустимы



