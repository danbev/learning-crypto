package example

import future.keywords

allow_if_has_fletch if {
  some p in input
    p.name = "Fletch"
}

default get_names = "No Name :("

get_names := name if {
  some p in input
    p.name == "Fletch"
    name := p.name
}

return_value = "yes" {
    1 == 1
}