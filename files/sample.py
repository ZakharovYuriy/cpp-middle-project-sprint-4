class AdvancedProcessor(SimpleProcessor):
    """Продвинутый процессор данных с дополнительной функциональностью"""
    
    def __init__(self, multiplier: float = 1.0, offset: float = 0.0):
        super().__init__(multiplier)
        self.offset = offset
    
    @log_execution(log_level=LogLevel.DEBUG)
    def process(self, data: List[Union[int, float]]) -> List[float]:
        """Расширенная обработка данных"""
        processed = super().process(data)
        return [x + self.offset for x in processed]
    
    def __call__(self, data: List[float]) -> List[float]:
        """Поддержка вызова как функции"""
        return self.process(data)


def lambda_demo():
    """Демонстрация использования лямбда-функций"""
    numbers = [1, 2, 3, 4, 5]
    
    # Лямбда для фильтрации
    even = filter(lambda x: x % 2 == 0, numbers)
    
    # Лямбда для преобразования
    squared = map(lambda x: x ** 2, numbers)
    
    # Лямбда для сортировки
    sorted_nums = sorted(numbers, key=lambda x: -x)
    
    return list(even), list(squared), sorted_nums