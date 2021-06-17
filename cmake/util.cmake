# Symlink Function
function(install_symlink target link)
    install(CODE "\
        # Prepare\n \
        set(file \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${link}\")\n \
        \
        # Create Directory\n \
        get_filename_component(dir \"\${file}\" DIRECTORY)\n \
        file(MAKE_DIRECTORY \${dir})\n \
        \
        # Create Symlink\n \
        if(NOT EXISTS \"\${file}\")\n \
            execute_process(COMMAND \${CMAKE_COMMAND} -E create_symlink ${target} \"\${file}\")\n \
            message(\"-- Installing: \${file}\")\n \
        else()\n \
            message(\"-- Up-to-date: \${file}\")\n \
        endif() \
    ")
endfunction()

