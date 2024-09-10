#include <vector>
#include <array>
#include <memory>
#include <thread>

#include <iostream>
#include <cstdlib>

#include <random>

#include "utility.hpp"

struct Coordinates
{
    Coordinates() = default;
    Coordinates(std::size_t x, std::size_t y) : m_x{x}, m_y{y} {}
    std::size_t m_x{0}, m_y{0};
};

using Dimensions = Coordinates;


using DamageUnits = std::size_t;
using HealthUnits = std::size_t;

class Unit
{
public:
    void hit(Unit *unit)
    {
        unit->take_damage(get_damage());
    }

    void set_default_hp()
    {
        m_hp = get_default_hp();
    }

protected:
    Unit() = default;
    

private:
    virtual DamageUnits get_damage() const = 0;
    virtual HealthUnits get_default_hp() const = 0;

    virtual void take_damage(DamageUnits dmg)
    {
        m_hp -= dmg;
    }

private:
    HealthUnits m_hp{1};
    
};

class MeleeAttacker : public Unit
{
public:
    MeleeAttacker()
    {
    }

private:
    DamageUnits get_damage() const override
    {
        return DamageUnits{5};
    }

    HealthUnits get_default_hp() const override
    {
        return HealthUnits{30};
    }

};

std::unique_ptr<MeleeAttacker> create_base_attacker()
{
    auto attacker {std::make_unique<MeleeAttacker>()};
    attacker->set_default_hp();
    return attacker;
}

enum class TileState
{
    Empty = 0, Unit
};


class Arena
{
public:
    void init_map(std::size_t width, std::size_t height)
    {
        if(width <= 0)
        {
            throw "Arena::init_map width is invalid";
        }
        if(height <= 0)
        {
            throw "Arena::init_map height is invalid";
        }
        

        m_width = width;
        m_height = height;

        m_tiles = std::vector<TileState>(m_width * m_height, TileState::Empty);
    }

    void place_unit(const Unit *unit, std::size_t pos)
    {
        m_tiles[pos] = TileState::Unit;
        m_units.push_back(unit);
        //m_units_pos.emplace_back(to_2d(pos));
        m_1d_units_pos.push_back(pos);
    }

    void move_unit(std::size_t unit_idx, std::size_t pos)
    {
        m_tiles[m_1d_units_pos[unit_idx]] = TileState::Empty;
        m_tiles[pos] = TileState::Unit;
        m_1d_units_pos[unit_idx] = pos;
    }

    std::size_t get_width() const { return m_width; }
    std::size_t get_height() const { return m_height; }

    std::size_t to_1d(const Coordinates &coord) const
    {
        return coord.m_y * get_width() + coord.m_x;
    }

    Coordinates to_2d(std::size_t idx) const
    {
        Coordinates ret {};
        ret.m_x = idx / get_height();
        ret.m_y = idx % get_height();
        return ret;
    }

    const std::vector<TileState> &get_tiles() const
    {
        return m_tiles;
    }

    const std::vector<std::size_t> &get_unit_pos() const
    {
        return m_1d_units_pos;
    }

    FixedSizeVector<std::size_t, 8> get_adjacent_unit_coordinates(std::size_t unit_idx) const
    {
        using std::cout;
        using std::endl;

        auto unit_pos {static_cast<long long int>(m_1d_units_pos[unit_idx])};
        std::cout << "unit_pos: " << unit_pos << std::endl;

        struct Obstacles
        {
            bool left:1 {0};
            bool right:1 {0};
            bool top:1 {0};
            bool bot:1 {0};

            bool lt_corner:1 {0};
            bool rt_corner:1 {0};
            bool lb_corner:1 {0};
            bool rb_corner:1 {0};
        };
        Obstacles wall_obst{}/*, other_obst{}*/;

        const auto tiles_size {static_cast<int>(m_tiles.size())};
        const auto width{static_cast<int>(get_width())};
        const auto height{static_cast<int>(get_height())};

        FixedSizeVector<std::size_t, 8> ret;

        if(unit_pos % get_width() == 0 )
        {
            cout << "hit a left wall\n";
            wall_obst.left = 1;
        }
        else if(unit_pos-1 >= 0 && m_tiles[unit_pos - 1] != TileState::Empty)
        {
            cout << "hit a left obstacle\n";
            //other_obst.left = 1;
        }
        else
        {
            cout << "can go left\n";
            ret.push_back(unit_pos-1);
        }
        if((unit_pos+1) % get_width() == 0)
        {
            cout << "hit a right wall\n";
            wall_obst.right = 1;
        }
        else if(unit_pos+1 <= tiles_size && m_tiles[unit_pos + 1] != TileState::Empty)
        {
            cout << "hit a right obstacle\n";
            //other_obst.right = 1;
        }
        else
        {
            cout << "can go right: " << unit_pos+1 << endl;
            ret.push_back(unit_pos+1);
        }
        if(unit_pos >= 0 && unit_pos <= width-1)
        {
            cout << "hit a top wall\n";
            wall_obst.top = 1;
        }
        else if(unit_pos - width >= 0 && m_tiles[unit_pos - width] != TileState::Empty)
        {
            cout << "hit a top obstacle\n";
            //other_obst.top = 1;
        }
        else
        {
            cout << "can go top: " << unit_pos - width << endl;
            ret.push_back(unit_pos - width);
        }
        if(unit_pos >= width * (height-1))
        {
            cout << "hit a bottom wall\n";
            wall_obst.bot = 1;
        }
        else if(unit_pos + width < tiles_size && m_tiles[unit_pos + width] != TileState::Empty)
        {
            cout << "hit a bottom obstacle\n";
            //other_obst.top = 1;
        }
        else
        {
            cout << "can go bottom: " << unit_pos + width << endl;
            ret.push_back(unit_pos + width);
        }
        
        if(!(wall_obst.left == 1 || wall_obst.top == 1) && unit_pos - width > 0 && m_tiles[unit_pos - width - 1] == TileState::Empty)
        {
            cout << "can go tl: " << unit_pos - width - 1 << endl;
            ret.push_back(unit_pos - width - 1);
        }
        if(!(wall_obst.right == 1 || wall_obst.top == 1) && unit_pos - width + 1 >= 0 && m_tiles[unit_pos - width + 1] == TileState::Empty)
        {
            cout << "can go tr: " << unit_pos - width + 1 << endl;
            ret.push_back(unit_pos - width + 1);
        }
        if(!(wall_obst.left == 1 || wall_obst.bot == 1) && unit_pos + width - 1 < tiles_size && m_tiles[unit_pos + width - 1] == TileState::Empty)
        {
            cout << "can go bl: " << unit_pos + width - 1 << endl;
            ret.push_back(unit_pos + width - 1);
        }
        if(!(wall_obst.right == 1 || wall_obst.bot == 1) && unit_pos + width + 1 < tiles_size && m_tiles[unit_pos + width + 1] == TileState::Empty)
        {
            cout << "can go br: " << unit_pos + width + 1 << endl;
            ret.push_back(unit_pos + width + 1);
        }

        return ret;
    }

private:
    std::size_t m_width {0}, m_height {0};

    std::vector<TileState> m_tiles;

    std::vector<const Unit *> m_units;
    //std::vector<Coordinates> m_units_pos;
    std::vector<std::size_t> m_1d_units_pos;    

};

class Level
{
public:
    void setup_level()
    {
        constexpr std::size_t width = 3;
        constexpr std::size_t height = 3;

        m_arena.init_map(width, height);
    }

    void setup_units()
    {
        constexpr std::size_t team_size {3};
        team_a.resize(team_size);


        // setup units on the opposide corners of each others
        //std::size_t idx {0};
        /*for(auto &itm : team_a)
        {
            itm = create_base_attacker();
            m_arena.place_unit(&*itm, idx++);
        }*/
        team_a[0] = create_base_attacker();
        m_arena.place_unit(&*team_a[0], 0);

        team_a[1] = create_base_attacker();
        m_arena.place_unit(&*team_a[1], 1);

        team_a[2] = create_base_attacker();
        m_arena.place_unit(&*team_a[2], 2);

        /*team_b.resize(team_size);
        idx = m_arena.get_width() * m_arena.get_height() - 1;
        for(auto &itm : team_b)
        {
            itm = create_base_attacker();
            m_arena.place_unit(&*itm, idx--);
        }*/
    }

    void depict()
    {
        //std::system("clear");
        static std::size_t iteration{0};
        std::cout << "It: " << iteration++ << std::endl;

        std::vector<std::vector<TileState>> res;

        const auto tiles {m_arena.get_tiles()};

        auto idx {0};
        for(const auto itm : tiles)
        {
            if(itm == TileState::Empty)
            {
                std::cout << " O ";
            }
            else if(itm == TileState::Unit)
            {
                std::cout << " \033[91mU\033[39m ";
            }
            if((idx + 1) % m_arena.get_width() == 0)
            {
                std::cout << "\n";
            }
            ++idx;
        }

    }

    void fiddle()
    {
        //auto units_count {m_arena.get_unit_pos().size()}
        const auto units_count {1};

        for(int i{0}; i < units_count;++i)
        {
            const auto possible_tiles {m_arena.get_adjacent_unit_coordinates(i)};
            const auto options {possible_tiles.size() - 1};

            std::random_device dev;
            std::mt19937 rng {dev()};
            std::uniform_int_distribution<std::mt19937::result_type> dist6 {0,options};
            m_arena.move_unit(i, possible_tiles[dist6(rng)]);
        }
    }

private:
    Arena m_arena;
    std::vector<std::shared_ptr<MeleeAttacker>> team_a;
    std::vector<std::shared_ptr<MeleeAttacker>> team_b;

};

int main()
{
    Level level;
    level.setup_level();
    level.setup_units();

    while(true)
    {
        level.depict();
        level.fiddle();
        //std::this_thread::sleep_for(std::chrono::seconds(2));
        getchar();
    }

    return 0;
}