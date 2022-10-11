package test_example

import future.keywords
import data.example.allow_if_has_fletch
import data.example.get_names

list := [{"name": "Fletch"}, {"name": "DrRosen"}, {"name": "MrSinilinden"}]

test_has_fletch if {
    allow_if_has_fletch with input as list
}

test_get_names if {
    get_names with input as list
}
