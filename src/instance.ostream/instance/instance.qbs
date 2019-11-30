import qbs

Project {
    name: "instance.ostream"
    minimumQbsVersion: "1.7.1"

    Product {
        name: "instance.ostream"

        Depends { name: "instance.data" }
        Depends { name: "parser.ostream" }

        files: [
            "Function.ostream.h",
            "Module.ostream.h",
            "Parameter.ostream.h",
            "Variable.ostream.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "instance.data" }
            Depends { name: "parser.ostream" }
        }
    }
}
