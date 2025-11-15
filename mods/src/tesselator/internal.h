#pragma once

#include <mods/tesselator/tesselator.h>
#include <libreborn/patch.h>

// Select Active Vertex Array
#define TEMPLATE \
    template <typename Vertex, VertexArray<Vertex> CustomTesselator::*VertexPtr>

// Patching
#define replace(name) \
    overwrite_call((void *) (name)->backup, name, name##_injection, true)
MCPI_INTERNAL void _tesselator_init_state();
MCPI_INTERNAL void _tesselator_init_vertex();
MCPI_INTERNAL void _tesselator_init_draw();