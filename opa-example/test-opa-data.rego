package test_example

import future.keywords
import data.example.allow_if_has_fletch
import data.example.get_names
import data.example.return_value

list := [{"name": "Fletch"}, {"name": "DrRosen"}, {"name": "MrSinilinden"}]

test_has_fletch if {
    allow_if_has_fletch with input as list
}

test_get_names if {
    names := get_names with input as list
    print("get_names returned:", names);
    names == "Fletch"
}

test_return_value if {
    r := return_value with input as "bajja"
    print("test_return_value returned:", r);
    r == "yes"
}
