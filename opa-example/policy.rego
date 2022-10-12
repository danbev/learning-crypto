package example

import future.keywords
import data.sample

allow_if_has_fletch if {
  some p in input
    p.name = "Fletch"
}

get_names := name if {
 some p in input
    p.name == "Fletch"
    name := p.name
}

return_value = "yes" {
    sample.number == 1
}

hello = "hello" {
    1 == 1
}

