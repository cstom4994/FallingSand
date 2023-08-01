// Metadot physics engine is enhanced based on box2d modification
// Metadot code Copyright(c) 2022-2023, KaoruXun All rights reserved.
// Box2d code by Erin Catto licensed under the MIT License
// https://github.com/erincatto/box2d

// MIT License
// Copyright (c) 2022-2023 KaoruXun
// Copyright (c) 2019 Erin Catto

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef B2_GEOMETRY_H
#define B2_GEOMETRY_H

#include "engine/physics/box2d/inc/b2_body.h"
#include "engine/physics/box2d/inc/b2_fixture.h"

void b2CreateLoop(b2Body* b, b2FixtureDef* fd, const b2Vec2* vertices, int32 count);
void b2CreateLoop(b2Body* b, const b2Vec2* vertices, int32 count);
void b2CreateChain(b2Body* b, b2FixtureDef* fd, const b2Vec2* vertices, int32 count, const b2Vec2 prevVertex, const b2Vec2 nextVertex);
void b2CreateChain(b2Body* b, const b2Vec2* vertices, int32 count, const b2Vec2 prevVertex, const b2Vec2 nextVertex);

#endif
