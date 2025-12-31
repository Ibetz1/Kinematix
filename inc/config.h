#ifndef _CONFIG_H
#define _CONFIG_H

/*
    psx config
*/
#define CFG_MAX_MANIFOLDS 8000
#define CFG_MAX_COLLIDERS 16000
#define CFG_MAX_SPACIALS  8000
#define CFG_MAX_MATERIALS 128
#define CFG_MAX_RAYS 64

#define CFG_FILL_ON_COLLIDE false
#define CFG_RENDER_BOUNDING_BOX false
#define CFG_RENDER_BVH false
#define CFG_MANIFOLDS_RENDER true
#define CFG_RENDER_FORCES false

#define CFG_DRAG_COEFFICIENT 2.f
#define CFG_ANG_DRAG_COEFFICIENT 2.f
#define CFG_INTERTIA_SCALAR 500.f

/*
    analytics
*/
#define CFG_ANALYTIC_LOG_INTERVAL 240
#define CFG_ENABLE_ANALYTICS false

/*
    draw config
*/
#define CFG_POINT_RADIUS 3.f
#define CFG_MAX_POLY_COMPLEXITY 4096

/*
    camera config
*/

#define CFG_MIN_ZOOM 0.1f
#define CFG_MAX_ZOOM 50.f

#endif