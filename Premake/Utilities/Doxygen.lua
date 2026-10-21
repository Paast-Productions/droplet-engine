local description =
[[ Generate Doxygen Files (Alias for "doxygen")
-----
]]

newaction {
    trigger = "doxygen",
    description = description,
    execute = function()
        os.execute("doxygen")
    end
}