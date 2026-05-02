# sample1.dsl — Basic arithmetic and control flow
# DSL Compiler Test | WASIM SHARAFATH M Y | RA2311026050171

program HelloWorld {
    var x : int = 10;
    var y : int = 20;
    var result : int;

    result = x + y;
    print(result);

    if (result > 25) {
        var msg : string = "Greater than 25";
        print(msg);
    } else {
        var msg2 : string = "Not greater";
        print(msg2);
    }

    var counter : int = 0;
    while (counter < 5) {
        print(counter);
        counter = counter + 1;
    }
} end
