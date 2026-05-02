# sample2.dsl — Functions and float arithmetic
# DSL Compiler Test | WASIM SHARAFATH M Y | RA2311026050171

program MathDemo {

    func add(a : int, b : int) -> int {
        return a + b;
    }

    func square(n : float) -> float {
        return n * n;
    }

    var a : int = 15;
    var b : int = 25;
    var sum : int;

    sum = add(a, b);
    print(sum);

    var pi : float = 3.14159;
    var radius : float = 5.0;
    var area : float;
    area = pi * square(radius);
    print(area);

    var i : int = 1;
    for (i = 1; i <= 10; i = i + 1) {
        print(i);
    }

} end
