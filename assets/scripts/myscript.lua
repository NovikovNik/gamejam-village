-- This is a Lua global variable
some_variable = 7 * 6

-- This is a Lua table that contain values for the initialization of our game engine
config = {
    title = "My Game Engine",
    fullscreen = false,
    resolution = {
        width = 800,
        height = 600
    }
}

-- This is a Lua function tat calculates the factorial of a give number
--
function factorial(n)
    if n < 0 or n == 1 then
        return 1
    end
    return n * factorial(n - 1)
end

factorial(5)

print("The cube of the number 3 is equal to " .. cube(3));
