#include "Fluid.hpp"

#include <cmath>	// std::floor

Fluid::Fluid()
    :
    s{ 0.0f },
    density{ 0.0f },
    Vx{ 0.0f },
    Vy{ 0.0f },
    Vx0{ 0.0f },
    Vy0{ 0.0f }
{
#if 0
    this->s = new float[N * N];
    this->density = new float[N * N];

    this->Vx = new float[N * N];
    this->Vy = new float[N * N];

    this->Vx0 = new float[N * N];
    this->Vy0 = new float[N * N];

    assert(s && density && Vx && Vy && Vx0 && Vy0);

    memset(s, 0, N * N);
    memset(density, 0, N * N);
    memset(Vx, 0, N * N);
    memset(Vy, 0, N * N);
    memset(Vx0, 0, N * N);
    memset(Vy0, 0, N * N);
#endif
}

void Fluid::Update() noexcept
{
    Diffuse(1, Vx0.data(), Vx.data(), VESCOSITY, MOTION_SPEED);
    Diffuse(2, Vy0.data(), Vy.data(), VESCOSITY, MOTION_SPEED);

    Project(Vx0.data(), Vy0.data(), Vx.data(), Vy.data());

    Advect(1, Vx.data(), Vx0.data(), Vx0.data(), Vy0.data(), MOTION_SPEED);
    Advect(2, Vy.data(), Vy0.data(), Vx0.data(), Vy0.data(), MOTION_SPEED);

    Project(Vx.data(), Vy.data(), Vx0.data(), Vy0.data());

    Diffuse(0, s.data(), density.data(), DIFFUSION, MOTION_SPEED);
    Advect(0, density.data(), s.data(), Vx.data(), Vy.data(), MOTION_SPEED);
}

void Fluid::AddDensity(int x, int y, float amount) noexcept
{
    this->density[IX(x, y)] += amount;
}

void Fluid::AddVelocity(int x, int y, float amountX, float amountY) noexcept
{
    const int index = IX(x, y);

    this->Vx[index] += amountX;
    this->Vy[index] += amountY;
}

void Fluid::Diffuse(int b, float* x, float* x0, float diff, float dt) noexcept
{
    const float a = dt * diff * (N - 2) * (N - 2);
    LinearSolve(b, x, x0, a, 1 + SCALE * a);
}

void Fluid::LinearSolve(int b, float* x, float* x0, float a, float c) noexcept
{
    const float cRecip = 1.0f / c;
    for (int k = 0; k < ITERATIONS; k++)
    {
        for (int j = 1; j < N - 1; j++)
        {
            for (int i = 1; i < N - 1; i++)
            {
                x[IX(i, j)] =
                    (x0[IX(i, j)]
                        + a * (x[IX(i + 1, j)]
                            + x[IX(i - 1, j)]
                            + x[IX(i, j + 1)]
                            + x[IX(i, j - 1)]
                            )) * cRecip;
            }
        }

        SetBoundary(b, x);
    }
}

void Fluid::SetBoundary(int b, float* x) noexcept
{
    for (int i = 1; i < N - 1; i++)
    {
        x[IX(i, 0)] = b == 2 ? -x[IX(i, 1)] : x[IX(i, 1)];
        x[IX(i, N - 1)] = b == 2 ? -x[IX(i, N - 2)] : x[IX(i, N - 2)];
    }
    for (int j = 1; j < N - 1; j++)
    {
        x[IX(0, j)] = b == 1 ? -x[IX(1, j)] : x[IX(1, j)];
        x[IX(N - 1, j)] = b == 1 ? -x[IX(N - 2, j)] : x[IX(N - 2, j)];
    }

    x[IX(0, 0)] = 0.5f * (x[IX(1, 0)] + x[IX(0, 1)]);
    x[IX(0, N - 1)] = 0.5f * (x[IX(1, N - 1)] + x[IX(0, N - 2)]);
    x[IX(N - 1, 0)] = 0.5f * (x[IX(N - 2, 0)] + x[IX(N - 1, 1)]);
    x[IX(N - 1, N - 1)] = 0.5f * (x[IX(N - 2, N - 1)] + x[IX(N - 1, N - 2)]);

}

void Fluid::Project(float* velocX, float* velocY, float* p, float* div) noexcept
{
    for (int j = 1; j < N - 1; j++) {
        for (int i = 1; i < N - 1; i++) {
            div[IX(i, j)] = -0.5f * (
                velocX[IX(i + 1, j)]
                - velocX[IX(i - 1, j)]
                + velocY[IX(i, j + 1)]
                - velocY[IX(i, j - 1)]
                ) / N;
            p[IX(i, j)] = 0;
        }
    }

    SetBoundary(0, div);
    SetBoundary(0, p);
    LinearSolve(0, p, div, 1, 4);

    for (int j = 1; j < N - 1; j++) {
        for (int i = 1; i < N - 1; i++) {
            velocX[IX(i, j)] -= 0.5f * (p[IX(i + 1, j)]
                - p[IX(i - 1, j)]) * N;
            velocY[IX(i, j)] -= 0.5f * (p[IX(i, j + 1)]
                - p[IX(i, j - 1)]) * N;
        }
    }
    SetBoundary(1, velocX);
    SetBoundary(2, velocY);

}

void Fluid::Advect(int b, float* d, float* d0, float* velocX, float* velocY, float dt) noexcept
{
    float i0, i1, j0, j1;

    const float dtx = dt * (N - 2);
    const float dty = dt * (N - 2);

    float s0, s1, t0, t1;
    float tmp1, tmp2, x, y;

    constexpr float Nfloat = static_cast<float>(N);
    float ifloat, jfloat;
    int i, j;

    for (j = 1, jfloat = 1; j < N - 1; j++, jfloat++)
    {
        for (i = 1, ifloat = 1; i < N - 1; i++, ifloat++)
        {
            const int index = IX(i, j);

            tmp1 = dtx * velocX[index];
            tmp2 = dty * velocY[index];

            x = ifloat - tmp1;
            y = jfloat - tmp2;

            if (x < 0.5f) x = 0.5f;
            if (x > Nfloat + 0.5f) x = Nfloat + 0.5f;
            i0 = std::floor(x);
            i1 = i0 + 1.0f;
            if (y < 0.5f) y = 0.5f;
            if (y > Nfloat + 0.5f) y = Nfloat + 0.5f;
            j0 = std::floor(y);
            j1 = j0 + 1.0f;


            s1 = x - i0;
            s0 = 1.0f - s1;
            t1 = y - j0;
            t0 = 1.0f - t1;


            int i0i = static_cast<int>(i0);
            int i1i = static_cast<int>(i1);
            int j0i = static_cast<int>(j0);
            int j1i = static_cast<int>(j1);

            d[index] =
                s0 * (t0 * d0[IX(i0i, j0i)] + t1 * d0[IX(i0i, j1i)]) +
                s1 * (t0 * d0[IX(i1i, j0i)] + t1 * d0[IX(i1i, j1i)]);

        }
    }
    SetBoundary(b, d);
}

Fluid::~Fluid() {}

/*





    std::unique_ptr<Fluid> m_fluid;
    olc::vf2d m_previous_mouse_pos;
    olc::Pixel m_fluid_color;
    float m_velocity_x;
    float m_velocity_y;







#include "FluidSimulation.h"
#include "../Utility/Random.hpp"

FluidSimulation::FluidSimulation(const std::string& title, int width, int height, int pixel_width, int pixel_height, bool full_screen, bool vsync)
{
    this->sAppName = title;

     this->Construct(width, height, pixel_width, pixel_height, full_screen, vsync);
}

bool FluidSimulation::OnUserCreate()
{
    m_fluid = std::make_unique<Fluid>();
    m_previous_mouse_pos = olc::vf2d{ 0.0f, 0.0f };
    m_fluid_color = olc::CYAN;
    m_velocity_x = m_velocity_y = 0.0f;

    return true;
}

bool FluidSimulation::OnUserInput(float fElapsedTime) noexcept
{
    // Handle exit on Escape pressed
    if (GetKey(olc::Key::ESCAPE).bPressed)
    {
        return false;
    }

    // Change color randomly on space pressed
    if (GetKey(olc::Key::SPACE).bPressed)
    {
        m_fluid_color = this->RandomColor();
    }


    // Handle mouse drag effect
    const float mouseX = static_cast<float>(GetMouseX());
    const float mouseY = static_cast<float>(GetMouseY());

    if (GetMouse(0).bHeld)
    {
        // Add some of dye
        m_fluid->AddDensity((int)mouseX / SCALE, (int)mouseY / SCALE, Random::Real(100.0f, 1000.0f));

        // Apply Mouse Drag Velocity
        const float amountX = (float)GetMouseX() - m_previous_mouse_pos.x;
        const float amountY = (float)GetMouseY() - m_previous_mouse_pos.y;
        m_fluid->AddVelocity(static_cast<int>(mouseX / SCALE), static_cast<int>(mouseY / SCALE), amountX, amountY);
    }


    // Apply velocity on arrow keys pressed
    //static constexpr const float VELOCITY_X = 0.0f; // no velocity applied in left or right side
    //static constexpr const float VELOCITY_Y = 0.0f; // +y to apply velocity to bottom and simulate fluid falling down
    static constexpr float VELOCITY = 0.03f;
    if (GetKey(olc::Key::UP).bHeld)
    {
        m_velocity_y += -VELOCITY;
    }
    else if (GetKey(olc::Key::DOWN).bHeld)
    {
        m_velocity_y += VELOCITY;
    }
    if (GetKey(olc::Key::RIGHT).bHeld)
    {
        m_velocity_x += VELOCITY;
    }
    else if (GetKey(olc::Key::LEFT).bHeld)
    {
        m_velocity_x += -VELOCITY;
    }
    if (GetKey(olc::Key::R).bPressed) // reset velocity
        m_velocity_x = m_velocity_y = 0.0f;


    // set previous mouse position
    m_previous_mouse_pos = { mouseX, mouseY };
    return true;
}


bool FluidSimulation::OnUserUpdate(float fElapsedTime)
{
    // Update Fluid
    m_fluid->Update();

    // Draw Fluid
    for (int j = 0; j < N; ++j)
    {
        for (int i = 0; i < N; ++i)
        {
            const int x = i * SCALE;
            const int y = j * SCALE;

            // Get Density 0 -> 255 alpha (background color)
            float& density = m_fluid->density[IX(i, j)];

            // Fix bug when color turn into black when adding too much density
            density = std::clamp(density, 0.0f, 255.0f);

            // Apply Fluid velocity
            m_fluid->AddVelocity(x, y, m_velocity_x * fElapsedTime, m_velocity_y * fElapsedTime);

            // Construct color based on density alpha
            m_fluid_color.a = static_cast<std::uint8_t>(density);

            // Draw pixel
            FillRect(x, y, SCALE, SCALE, m_fluid_color);
        }
    }

    // Clamp velocity
    m_velocity_x = std::clamp(m_velocity_x, -0.15f, 0.15f);
    m_velocity_y = std::clamp(m_velocity_y, -0.15f, 0.15f);

    return this->OnUserInput(fElapsedTime); // Handle user input & check for escape press to exit
}


bool FluidSimulation::OnUserDestroy()
{

    return true;
}




*/