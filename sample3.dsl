# sample3.dsl — Boolean logic and nested conditions
# DSL Compiler Test | WASIM SHARAFATH M Y | RA2311026050171

program BoolLogic {

    var flag1 : bool = true;
    var flag2 : bool = false;
    var result : bool;

    result = flag1 && !flag2;

    if (result) {
        var msg : string = "Both conditions met";
        print(msg);
    }

    var score : int = 85;

    if (score >= 90) {
        var grade : string = "A";
        print(grade);
    } else {
        if (score >= 75) {
            var grade2 : string = "B";
            print(grade2);
        } else {
            var grade3 : string = "C";
            print(grade3);
        }
    }

} end
