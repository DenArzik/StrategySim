#include <vector>
#include <memory>
#include <thread>

#include <iostream>
#include <cstdlib>
#include <limits>

#include "utility.hpp"

using SizeT = std::size_t;

struct Coordinates
{
    Coordinates() = default;
    Coordinates(SizeT x, SizeT y) : m_x{x}, m_y{y} {}
    SizeT m_x{0}, m_y{0};
};

using Dimensions = Coordinates;

using DamageUnits = SizeT;
using HealthUnits = SizeT;


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
    void init_map(SizeT width, SizeT height)
    {
        if(width < 2)
        {
            throw "Arena::init_map width is invalid";
        }
        if(height < 2)
        {
            throw "Arena::init_map height is invalid";
        }
        

        m_width = width;
        m_height = height;

        m_tiles = std::vector<TileState>(m_width * m_height, TileState::Empty);
    }

    void place_unit(const Unit *unit, SizeT pos)
    {
        m_tiles[pos] = TileState::Unit;
        m_units.push_back(unit);
        //m_units_pos.emplace_back(to_2d(pos));
        m_1d_units_pos.push_back(pos);
    }

    void move_unit(SizeT unit_idx, SizeT pos)
    {
        m_tiles[m_1d_units_pos[unit_idx]] = TileState::Empty;
        m_tiles[pos] = TileState::Unit;
        m_1d_units_pos[unit_idx] = pos;
    }

    SizeT get_width() const { return m_width; }
    SizeT get_height() const { return m_height; }

    SizeT to_1d(const Coordinates &coord) const
    {
        return coord.m_y * get_width() + coord.m_x;
    }

    Coordinates to_2d(SizeT idx) const
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

    const std::vector<SizeT> &get_unit_pos() const
    {
        return m_1d_units_pos;
    }

    FixedSizeVector<SizeT, 8> get_adjacent_unit_coordinates(SizeT unit_idx) const
    {
        using std::cout;
        using std::endl;

        auto unit_pos {m_1d_units_pos[unit_idx]};
        //std::cout << "unit_pos: " << unit_pos << std::endl;

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

        const auto tiles_size {m_tiles.size()};
        const auto width{get_width()};
        const auto height{get_height()};

        FixedSizeVector<SizeT, 8> ret;

        // left check
        if(unit_pos % width == 0)
        {
            // cout << "hit a left wall\n";
            wall_obst.left = 1;
        }
        else if(unit_pos > 0)
        {
            if(m_tiles[unit_pos - 1] != TileState::Empty)
            {
                // cout << "hit a left obstacle\n";
                //other_obst.left = 1;
            }
            else
            {
                // cout << "can go left\n";
                ret.push_back(unit_pos-1);
            }
        }

        // right check
        if(unit_pos < tiles_size)
        {
            if((unit_pos+1) % width == 0)
            {
                // cout << "hit a right wall\n";
                wall_obst.right = 1;
            }
            else if(m_tiles[unit_pos + 1] != TileState::Empty)
            {
                // cout << "hit a right obstacle\n";
                //other_obst.right = 1;
            }
            else
            {
                // cout << "can go right: " << unit_pos+1 << endl;
                ret.push_back(unit_pos+1);
            }
        }

        // top check
        if(unit_pos < width)
        {
            // cout << "hit a top wall\n";
            wall_obst.top = 1;
        }
        else if(unit_pos >= width)
        {
            if(m_tiles[unit_pos - width] != TileState::Empty)
            {
                // cout << "hit a top obstacle\n";
                //other_obst.top = 1;
            }
            else
            {
                // cout << "can go top: " << unit_pos - width << endl;
                ret.push_back(unit_pos - width);
            }
        }
        
        // bot check
        if(unit_pos >= width * (height-1))
        {
            // cout << "hit a bottom wall\n";
            wall_obst.bot = 1;
        }
        else if(tiles_size - width > unit_pos)
        {
            if(m_tiles[unit_pos + width] != TileState::Empty)
            {
                // cout << "hit a bottom obstacle\n";
                //other_obst.top = 1;
            }
            else
            {
                // cout << "can go bottom: " << unit_pos + width << endl;
                ret.push_back(unit_pos + width);
            }
        }
        
        if(!(wall_obst.left == 1 || wall_obst.top == 1) && unit_pos > width && m_tiles[unit_pos - width - 1] == TileState::Empty)
        {
            // cout << "can go tl: " << unit_pos - width - 1 << endl;
            ret.push_back(unit_pos - width - 1);
        }
        if(!(wall_obst.right == 1 || wall_obst.top == 1) && unit_pos >= width && m_tiles[unit_pos - width + 1] == TileState::Empty)
        {
            // cout << "can go tr: " << unit_pos - width + 1 << endl;
            ret.push_back(unit_pos - width + 1);
        }
        if(!(wall_obst.left == 1 || wall_obst.bot == 1) && tiles_size - width > unit_pos && m_tiles[unit_pos + width - 1] == TileState::Empty)
        {
            // cout << "can go bl: " << unit_pos + width - 1 << endl;
            ret.push_back(unit_pos + width - 1);
        }
        if(!(wall_obst.right == 1 || wall_obst.bot == 1) && tiles_size - width - 1 > unit_pos && m_tiles[unit_pos + width + 1] == TileState::Empty)
        {
            // cout << "can go br: " << unit_pos + width + 1 << endl;
            ret.push_back(unit_pos + width + 1);
        }

        return ret;
    }

private:
    SizeT m_width {0}, m_height {0};

    std::vector<TileState> m_tiles;

    std::vector<const Unit *> m_units;
    std::vector<SizeT> m_1d_units_pos;    

};

class Level
{
public:
    void setup_level()
    {
        constexpr SizeT width = 5;
        constexpr SizeT height = 5;

        // size checks
        static_assert(width > 1);
        static_assert(height > 1);
        static_assert(width < (std::numeric_limits<SizeT>::max() / height));

        m_arena.init_map(width, height);
    }

    void setup_units()
    {
        constexpr std::size_t team_size {5};
        if(team_size > (m_arena.get_width() * m_arena.get_height()) / 2)
        {
            throw "team_size too large";
        }

        team_a.resize(team_size);


        // setup units on the opposide corners of each others
        std::size_t idx {0};
        for(auto &itm : team_a)
        {
            itm = create_base_attacker();
            m_arena.place_unit(&*itm, idx++);
        }

        team_b.resize(team_size);
        idx = m_arena.get_width() * m_arena.get_height() - 1;
        for(auto &itm : team_b)
        {
            itm = create_base_attacker();
            m_arena.place_unit(&*itm, idx--);
        }
    }

    void depict()
    {
        std::system("clear");
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
        const auto units_count {m_arena.get_unit_pos().size()};
        //const auto units_count {1};

        for(std::size_t i{0}; i < units_count;++i)
        {
            const auto possible_tiles {m_arena.get_adjacent_unit_coordinates(i)};
            
            if(possible_tiles.empty())
            {
                continue;
            }
            if(possible_tiles.size() == 1)
            {
                m_arena.move_unit(i, possible_tiles[0]);
                continue;
            }
            const auto options {possible_tiles.size() - 1};
            const auto rand_int {RNG::Generator<>::get().random_uniform_uint(0ul, options)};
            m_arena.move_unit(i, possible_tiles[rand_int]);
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
        getchar();
    }

    return 0;
}