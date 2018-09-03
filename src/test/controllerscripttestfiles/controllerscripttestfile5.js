;(function (global) {
    include("controllerscripttestfile5.js")
    if (global.counter === undefined) {
        global.counter = 0;
    }
    global.counter += 1;
}(this));
