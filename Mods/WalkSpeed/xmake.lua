local projectName = "DynamicMovementWalkSpeed"
local projectVersion = "1.0.0"

add_requires("toml++")

target(projectName)
    set_version(projectVersion)
    add_rules("ue4ss.mod")
    set_languages("cxx23")
    add_packages("toml++")
    add_includedirs("../../Common")
    add_files("../../Common/**.cpp")
    add_includedirs("Source")
    add_files("Source/**.cpp")
    add_defines("MOD_NAME=\"" .. projectName .. "\"")
    add_defines("MOD_VERSION=\"" .. projectVersion .. "\"")

xpack(projectName)
    set_version(projectVersion)
    set_formats("zip")
    set_prefixdir("OblivionRemastered/Binaries/Win64/ue4ss/Mods/" .. projectName)

    on_load(function (package)
        package.outputdir = function() return "Distribution" end
    end)

    before_package(function (package)
        import("core.project.project")
        target = project.target(projectName)
        targetdir = target:get("targetdir")

        enabled_file_path = path.join(os.tmpdir(), "enabled.txt")
        local enabled_file = io.open(enabled_file_path, "w")
        if enabled_file then
            enabled_file:close()
        else
            os.raise("failed to create file: %s", enabled_file_path)
        end

        package:add("installfiles", enabled_file_path)
        package:add("installfiles", targetdir .. "/**.dll", {prefixdir = "DLLs"})
    end)
