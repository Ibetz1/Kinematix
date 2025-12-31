#ifndef _ANALYTICS_H
#define _ANALYTICS_H

#include "main.h"

#if CFG_ENABLE_ANALYTICS

struct ColliderAnalytics {
    static U32 current_frame_count;
    static U32 sanity_check_iterations;
    static U32 parition_iterations;
    static U32 build_recursive_iterations;
    static U32 narrow_phase_checks;
    static U32 missed_narrow_phase_checks;
    static U32 broad_phase_checks;
    static U32 bvh_traversal_count;
    static U32 manifolds_created;
    static U32 heap_allocated_bytes;

    static void clean() {
        if ((current_frame_count % CFG_ANALYTIC_LOG_INTERVAL)) return;

        sanity_check_iterations = 0;
        parition_iterations = 0;
        build_recursive_iterations = 0;
        narrow_phase_checks = 0;
        missed_narrow_phase_checks = 0;
        broad_phase_checks = 0;
        bvh_traversal_count = 0;
        manifolds_created = 0;
    }

    static void log() {
        if ((current_frame_count % CFG_ANALYTIC_LOG_INTERVAL)) return;

        printf("---\n");
        printf("sanity_check_iterations = %i\n", sanity_check_iterations);
        printf("parition_iterations = %i\n", parition_iterations);
        printf("build_recursive_iterations = %i\n", build_recursive_iterations);
        printf("narrow_phase_checks = %i\n", narrow_phase_checks);
        printf("missed_narrow_phase_checks = %i\n", missed_narrow_phase_checks);
        printf("broad_phase_checks = %i\n", broad_phase_checks);
        printf("bvh_traversal_count = %i\n", bvh_traversal_count);
        printf("manifolds_created = %i\n", manifolds_created);
        printf("heap_allocated_bytes = %i\n", heap_allocated_bytes);
    }
};

#endif

#endif