project(sd_pico)

add_library(common_stuff_sd_pico INTERFACE)
target_link_libraries(common_stuff_sd_pico INTERFACE
        pico_stdlib
        pico_sdio
        )

add_library(sd_pico STATIC fatfs_sdio.c)
target_link_libraries(sd_pico common_stuff_sd_pico)