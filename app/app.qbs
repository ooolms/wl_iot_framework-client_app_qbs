import qbs

CppApplication
{
	Depends {name: "Qt"; submodules: ["core"]}
	Depends {name: "libwliotproxy-static"}

	files:[
        "CmdArgParser.cpp",
        "CmdArgParser.h",
        "main.cpp",
    ]
}
