

#ifndef INC_COLLISION_CIRCLE_RECT_H
#define INC_COLLISION_CIRCLE_RECT_H

#include "Poro/utils/math/math_utils.h"
#include "Poro/utils/rect/crect_functions.h"
#include <vector>

namespace CEngine {

    template<class VectorT, class RectT>
    bool CircleRectCollide(const VectorT &circle_p, float circle_r, const RectT &rect) {
        if (CEngine::RectHit(RectT(circle_p.x - circle_r, circle_p.y - circle_r, circle_r * 2.f,
                                circle_r * 2.f),
                          rect) == false) {
            return false;
        }

        // go through the lines of the box and check collision agains them
        std::vector<VectorT> rect_p(4);
        rect_p[0].Set(rect.x, rect.y);
        rect_p[1].Set(rect.x + rect.w, rect.y);
        rect_p[2].Set(rect.x + rect.w, rect.y + rect.h);
        rect_p[3].Set(rect.x, rect.y + rect.h);

        const float circle_r_squared = CEngine::math::Square(circle_r);
        float dist = 0;
        for (std::size_t i = 0; i < rect_p.size(); ++i) {
            dist = CEngine::math::DistanceFromLineSquared(rect_p[i], rect_p[(i + 1) % 4], circle_p);
            if (dist <= circle_r_squared) return true;
        }

        return false;
    }

    template<class VectorT, class RectT>
    VectorT CircleRectResolveByPushingCircle(const VectorT &circle_p, float circle_r,
                                             const RectT &rect) {
        // go through the lines of the box and check collision agains them
        // we go through the lines and we find the closest position that we collide with
        std::vector<VectorT> rect_p(4);
        rect_p[0].Set(rect.x, rect.y);
        rect_p[1].Set(rect.x + rect.w, rect.y);
        rect_p[2].Set(rect.x + rect.w, rect.y + rect.h);
        rect_p[3].Set(rect.x, rect.y + rect.h);

        float dist = 0;
        VectorT point;
        float closest_dist = CEngine::math::Square(circle_r);
        VectorT closest_point;
        for (std::size_t i = 0; i < rect_p.size(); ++i) {
            point = CEngine::math::ClosestPointOnLineSegment(rect_p[i], rect_p[(i + 1) % 4], circle_p);
            dist = (circle_p - point).LengthSquared();

            if (dist < closest_dist) {
                closest_dist = dist;
                closest_point = point;
            }
        }

        const float extra_push = 0.1f;

        VectorT delta = circle_p - closest_point;
        delta = (delta.Normalize() * (circle_r + extra_push));

        // to fix it so that if the point is inside the box it's pushed outside
        if (CEngine::math::IsPointInsideAABB(circle_p, rect_p[0], rect_p[2])) delta = -delta;

        return closest_point + delta;
    }

}// end of namespace CEngine
#endif
