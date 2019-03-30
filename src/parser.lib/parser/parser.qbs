import qbs

Project {
    name: "parser.lib"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "parser.lib"
        targetName: "parser"

        Depends { name: "instance.data" }
        Depends { name: "diagnostic.data" }

        files: [
            "Context.cpp",
            "Context.h",
            "LineErrorReporter.cpp",
            "LineErrorReporter.h",
            "LineView.h",
            "Parser.cpp",
            "Parser.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "instance.data" }
            Depends { name: "diagnostic.data" }
        }
    }

    Application {
        name: "parser.tests"
        consoleApplication: true
        type: base.concat("autotest")

        Depends { name: "parser.lib" }
        Depends { name: "parser.ostream" }
        Depends { name: "instance.ostream" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "expressionParser.test.cpp",
        ]
    }
}
