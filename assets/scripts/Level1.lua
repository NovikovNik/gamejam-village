Level = {
    assets = {
        [0] =
        { type = "texture", id = "tilemap-image", file = "./assets/tilemaps/jungle.png" },
        { type = "texture", id = "chopper-image", file = "./assets/images/chopper-spritesheet.png" },
        { type = "texture", id = "radar",         file = "./assets/images/radar.png" },
        { type = "texture", id = "truck-image",   file = "./assets/images/truck-ford-right.png" },
        { type = "texture", id = "tank-image",    file = "./assets/images/tank-panther-right.png" },
        { type = "texture", id = "bullet-image",  file = "./assets/images/bullet.png" },
        { type = "texture", id = "tree-image",    file = "./assets/images/tree.png" },
        { type = "font",    id = "pico8",         file = "./assets/fonts/pico-8.ttf",              size = 20 },
        { type = "font",    id = "pico8-hp",      file = "./assets/fonts/pico-8.ttf",              size = 8 },
        { type = "font",    id = "arial",         file = "./assets/fonts/arial.ttf",               size = 10 }
    },
    tilemap = "./assets/tilemaps/jungle.map",
    entities = {
        [0] =
        {
            tag = "Player",
            components = {
                transform_component = {
                    x = 100,
                    y = 100,
                    scale_x = 1,
                    scale_y = 1,
                    angle = 0
                },
                rigidbody_component = {
                    x = 0.0,
                    y = 0.0
                },
                sprite_component = {
                    id = "chopper-image",
                    z_index = 2,
                    h = 32,
                    w = 32,
                    is_fixed = false
                },
                emitter_component = {
                    speed = 150,
                    frequency = 500,
                    duration = 10000,
                    damage = 10,
                    is_friendly = true,
                    auto_fire = false
                },
                animation_component = {
                    frames = 2,
                    curr_frame = 0,
                    speed = 15,
                    is_loop = true
                },
                control_component = {
                    up_velocity = {
                        x = 0,
                        y = -80
                    },
                    right_velocity = {
                        x = 80,
                        y = 0
                    },
                    down_velocity = {
                        x = 0,
                        y = 80
                    },
                    left_velocity = {
                        x = -80,
                        y = 0
                    }
                },
                camera_component = true,
                boxcollider_component = {
                    size = {
                        x = 32,
                        y = 32
                    },
                    offset = {
                        x = 0,
                        y = 0
                    }
                },
                health_component = {
                    value = 100
                },
                healthbar_component = true,
            }
        },
        {
            -- text label for level name
            components = {
                label_component = {
                    position = {
                        x = 100,
                        y = 300
                    },
                    text = "Hi!",
                    font = "pico8",
                    color = "white"
                },
                on_update_script = {
                    [0] = function(entity, delta_time, ellapsed_time)
                        print("Executing comand")
                    end
                }
            }
        },
        {
            group = "enemies",
            components = {
                transform_component = {
                    x = 280,
                    y = 380,
                    scale_x = 1,
                    scale_y = 1,
                    angle = 0
                },
                rigidbody_component = {
                    x = 0.0,
                    y = 100.0
                },
                boxcollider_component = {
                    size = {
                        x = 32,
                        y = 32
                    },
                    offset = {
                        x = 0,
                        y = 0
                    }
                },
                sprite_component = {
                    id = "truck-image",
                    z_index = 2,
                    h = 32,
                    w = 32,
                    is_fixed = false
                },
                emitter_component = {
                    speed = 150,
                    frequency = 2500,
                    duration = 10000,
                    damage = 10,
                    is_friendly = false,
                    auto_fire = true
                },
                health_component = {
                    value = 100
                },
                healthbar_component = true,
                on_update_script = {
                    [0] = function(entity, delta_time, ellapsed_time)
                        local current_position_x, current_position_y = get_position(entity)
                        local current_velocity_x, current_velocity_y = get_velocity(entity)

                        if (current_position_y < 10 or current_position_y > 800 - 32) then
                            set_velocity(entity, 0, current_velocity_y * -1)
                        else
                            set_velocity(entity, 0, current_velocity_y)
                        end
                    end
                }
            }
        },
    }
}
