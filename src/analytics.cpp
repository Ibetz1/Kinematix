#include "analytics.h"

#if CFG_ENABLE_ANALYTICS

U32 ColliderAnalytics::heap_allocated_bytes = 0;
U32 ColliderAnalytics::current_frame_count = 0;
U32 ColliderAnalytics::sanity_check_iterations = 0;
U32 ColliderAnalytics::parition_iterations = 0;
U32 ColliderAnalytics::build_recursive_iterations = 0;
U32 ColliderAnalytics::narrow_phase_checks = 0;
U32 ColliderAnalytics::missed_narrow_phase_checks = 0;
U32 ColliderAnalytics::broad_phase_checks = 0;
U32 ColliderAnalytics::bvh_traversal_count = 0;
U32 ColliderAnalytics::manifolds_created = 0;

#endif