#include "gl_express_shape.h"

bool glx_aabb_check(const GlxBoundingBox& a, const GlxBoundingBox& b) {
    if (a.max.x < b.min.x) return false;
    if (a.min.x > b.max.x) return false;
    if (a.max.y < b.min.y) return false;
    if (a.min.y > b.max.y) return false;
    return true;
}

AABB glx_aabb_merge(const AABB& a, const AABB& b) {
    AABB out;
    out.min.x = fminf(a.min.x, b.min.x);
    out.min.y = fminf(a.min.y, b.min.y);
    out.max.x = fmaxf(a.max.x, b.max.x);
    out.max.y = fmaxf(a.max.y, b.max.y);
    return out;
}

Vec2 glx_aabb_center(const AABB& bb) {
    return (bb.min + bb.max) * 0.5f;
}

F32 glx_aabb_perimeter(const AABB& a) {
    F32 wx = a.max.x - a.min.x;
    F32 wy = a.max.y - a.min.y;
    return 2.0f * (wx + wy);
}

Status glx_shape_2d(GLX& glx, const F32* xy, S32 count) {
    if (count < 2) return ERROR;
    S32 bytes = count * 2 * sizeof(F32);

    glBufferData(GL_ARRAY_BUFFER, bytes, xy, GL_DYNAMIC_DRAW);

    switch(glx.draw_mode) {
        case(DRAW_FILL) : { 
            glDrawArrays(GL_TRIANGLE_FAN, 0, count);
            break; 
        }

        case(DRAW_LINES) : { 
            if (count == 2) {
                glDrawArrays(GL_LINE_STRIP, 0, count);   // <--- draw simple line segment
            } else {
                glDrawArrays(GL_LINE_LOOP, 0, count);
            }
            break;
        }
    }

    return OK;
}

void glx_rotate_poly_2d(const F32* in_verts, F32* out_verts, U32 vert_count, F32 cx, F32 cy, F32 angle) {
    F32 s = sinf(angle);
    F32 c = cosf(angle);

    for (U32 i = 0; i < vert_count; ++i) {
        F32 px = in_verts[i * 2 + 0];
        F32 py = in_verts[i * 2 + 1];

        // translate to center
        F32 dx = px - cx;
        F32 dy = py - cy;

        // rotate
        F32 rx = dx * c - dy * s;
        F32 ry = dx * s + dy * c;

        // translate back
        out_verts[i * 2 + 0] = cx + rx;
        out_verts[i * 2 + 1] = cy + ry;
    }
}

Status glx_rect_2d(GLX& glx, F32 x, F32 y, F32 w, F32 h, F32 angle) {

    // If no rotation, just use the axis-aligned rect for speed
    F32 verts[] = {
        x,     y,
        x + w, y,
        x + w, y + h,
        x,     y + h
    };

    if (angle != 0.f) {
        // center of the rectangle
        F32 cx = x + w * 0.5f;
        F32 cy = y + h * 0.5f;

        F32 rotated[8];
        glx_rotate_poly_2d(verts, rotated, 4, cx, cy, angle);
        return glx_shape_2d(glx, rotated, 4);
    }

    return glx_shape_2d(glx, verts, 4);
}

Status glx_circle_2d(GLX& glx, F32 x, F32 y, F32 r, S32 lod) {

    /*
        build circle specs
    */

    constexpr U32 min_lod = 10;
    constexpr U32 max_lod = 512;
    constexpr F32 auto_lod = 6.f; // 6px per arc unit (autoscaling only)

    // auto resolution scaling
    if (lod == -1) {
        F32 circumference = 2.f * M_PI * r;
        lod = (U32) (circumference / auto_lod);
    }
    
    if (lod < 8) lod = min_lod;
    if (lod >= max_lod) lod = max_lod;

    /*
        build the circle
    */

    static F32 vertex_buffer[1 + max_lod * 2] = { 0.f };
    
    F32 normal = 0.f;

    F32 step = (2 * M_PI) / (F32) lod;
    const F32 tcos = cosf(step);
    const F32 tsin = sinf(step);

    S32 index = 0;
    F32 vx = r;
    F32 vy = 0.f;

    for (S32 i = 0; i < lod; ++i) {
        F32 angle = i * step;
        vertex_buffer[index++] = x + vx;
        vertex_buffer[index++] = y + vy;
        
        const F32 nx = tcos * vx - tsin * vy;
        const F32 ny = tsin * vx + tcos * vy;

        vx = nx; 
        vy = ny;
    }

    Status ret = glx_shape_2d(glx, vertex_buffer, lod);
    return ret;
}

void glx_transform_poly_2d(
    Vec2 pos, 
    Vec2* out, 
    const Vec2* identity, 
    U32 count, 
    F32 scale, 
    F32 angle, 
    GlxBoundingBox* bounding_box,
    Vec2* center
) {
    F32 s = 0.f;
    F32 c = 0.f;
    if (angle != 0.f) {
        s = sinf(angle);
        c = cosf(angle);
    }

    bool set_center = center != nullptr;
    bool set_bounding_box = bounding_box != nullptr;

    if (set_center) *center = {0, 0};

    for (U32 i = 0; i < count; ++i) {
        if (angle == 0.f) {
            out[i] = (identity[i] * scale) + pos;
        } else {
            out[i] = vec2_rotate((identity[i] * scale) + pos, pos, s, c);
        }

        if (set_bounding_box) {
            if (out[i].x < bounding_box->min.x) bounding_box->min.x = out[i].x;
            if (out[i].y < bounding_box->min.y) bounding_box->min.y = out[i].y;
            if (out[i].x > bounding_box->max.x) bounding_box->max.x = out[i].x;
            if (out[i].y > bounding_box->max.y) bounding_box->max.y = out[i].y;
        }

        if (set_center) *center += out[i];
    }

    if (set_center) *center /= (F32) count; 
}