# AUTOGEN

Autogen is a tool used in this project to automatically fill in reflection metadata, making it easy to interface with Instances dynamically at runtime.

The goal is to minimize manual boilerplate by having Autogen generate it for us. Currently, it is used to generate Instance metadata and property metadata.
In the future, it will also be used to generate Lua interfaces, etc.

## How to use

### In CMake

Autogen operates on a directory of files. First, collect all the header files in the directory you want to parse

    # Run autogen
    file(GLOB_RECURSE AUTOGEN_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/src/objects" "src/objects/*.h")

Then, for each source, derive an `OUT_PATH` wherein each file now ends in ".cpp" and is in the desired output directory.
Now, you want to run `add_custom_command` to call autogen with the sources root, source path, and output path for the generated file.
Don't forget to set the `OUTPUT` and `DEPENDS` properties of `add_custom_command`.
Then, add each `OUT_PATH` to a list, `AUTOGEN_OUTS`

    foreach (SRC ${AUTOGEN_SOURCES})
        string(REGEX REPLACE "[.]h$" ".cpp" OUT_SRC_NAME ${SRC})
        set(SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/objects/${SRC}")
        set(OUT_PATH "${CMAKE_BINARY_DIR}/generated/${OUT_SRC_NAME}")

        add_custom_command(
            OUTPUT "${OUT_PATH}"
            DEPENDS "${SRC_PATH}"
            COMMAND "${CMAKE_BINARY_DIR}/autogen/autogen" "${CMAKE_CURRENT_SOURCE_DIR}/src" "${SRC_PATH}" "${OUT_PATH}"
        )

        list(APPEND AUTOGEN_OUTS "${OUT_PATH}")
    endforeach()

Finally, create a build target that depends on your `AUTOGEN_OUTS`, and make `ALL` depend on it

    add_custom_target(autogen_build ALL
        DEPENDS ${AUTOGEN_OUTS}
    )

### In Code

Autogen is an annotation processor, it largely only interacts with C++ annotations (`[[ ... ]]`).
It uses libClang to parse through the header and look for classes annotated with `OB::def_inst`. In `annotation.h`, there are various aliases for this command:

- `INSTANCE` - Simply annotate with `def_inst`, do not do anything more. Equivalent to `[[ def_inst() ]]`
- `INSTANCE_WITH(...)` - Same as above, but allows you to pass in arguments to `def_inst`. Equivalent to `[[ def_inst(...) ]]`
- `INSTANCE_SERVICE(...)` - Same as above, but adds `, service` to the end of the argument list, marking the instance as a service.
Equivalent to `[[ def_inst(..., service) ]]`

Then, there is the command itself:

    def_inst(abstract?, not_creatable?, service?, explorer_icon=?)  

It comes with various parameters/flags:

- `abstract` - Flag, the class is a base class for other instances, and cannot/should not be constructed (Autogen will set `.constructor` to `nullptr`)
- `not_creatable` - Flag, the instance is not creatable via code/APIs, only internally. (Adds the `INSTANCE_NOTCREATABLE` flag)
- `service` - Flag, the instance is a service. Additionally, enable the `not_creatable` flag as well (Adds both `INSTANCE_NOTCREATABLE` and `INSTANCE_SERVICE` flags)
- `explorer_icon` - Option, string to pass into editor for the icon of the instance in the explorer

A file will be generated in `build/generated/<header>.cpp` with the `TYPE` automatically filled in, as well as an implementation for `GetClass()`

Note that it is also necessary to add `AUTOGEN_PREAMBLE` to the top of your class definition, to make sure that various functions for properties, etc. are
declared such that they can later be defined in the generated source file.

Properties are also a key component. Inside every class annotated with `def_inst`, the analyzer will scan for any `OB::def_prop` annotations as well.

    def_prop(name=, hidden?, no_save?, unit_float?, readonly?, category=?, on_update=?)

Here are its parameters:

- `name` - Required parameter, this is the name of the property itself. The name of the field is not taken into consideration (subject to change)
- `hidden` - Flag, marks the property as hidden from the editor.
- `no_save` - Flag, the property should not be deserialized nor serialized
- `readonly` - Flag, the property cannot be assigned to
- `category` - Option, the category the property will appear in in the editor. Accepted values are: `DATA` (default), `APPEARANCE`, `PART`, `BEHAVIOR`, `SURFACE`
- `on_update` - Option, callback to call after the property has been assigned to. Should accept a std::string containing the property name and return void

The type of the property, and conversion to and from it and the datatype system is automatically inferred. `std::string` is interpreted as `Data::String`, and `std::weak_ptr<T>` is also converted to/from `Data::InstanceRef`. In the future, if weird edge-case types are introduced, the code generator may need to be extended. See [Extending Autogen](#extending-autogen)

In Part, it is necessary to expose the position and rotation components of the CFrame as properties, so there are two commands for this case (these should be added alongside `def_prop` on the CFrame property):

    cframe_position_prop(name=)
    cframe_rotation_prop(name=)

They simply create properties of the component with the specified name. They will inherit the category and update callback from the CFrame property, and will be flagged with NOSAVE. A getter/setter is automatically generated which generates a new CFrame with the specific component changed for you.

### Example

Here is an example of an instance annotated using Autogen

    class INSTANCE Part {
        AUTOGEN_PREAMBLE
    public:
        [[ def_prop(name="Color") ]]
        Color3 color;

        [[ def_prop(name="CFrame"), cframe_position_prop(name="Position") ]]
        CFrame cframe;

        [[ def_prop(name="ReadOnly", readonly) ]]
        int readOnlyValue;

        [[ def_prop(name="Ephemeral", no_save) ]]
        std::string ephemeral;

        [[ def_prop(name="SuperSecretValue", no_save, hidden) ]]
        std::string superSecret;
    };

# Extending Autogen

WIP.