#include "CharacterManager.hpp"
#include <limits>

class CharacterManager::Impl
{
public:
    Impl()
        : characters_(),
          my_character_id_(std::numeric_limits<decltype(my_character_id_)>::max())
    {}

    void Add(unsigned int character_id, CharacterPtrType character)
    {
        characters_.insert(std::make_pair(character_id, character));
    }

    CharacterPtrType Get(unsigned int character_id) const
    {
        auto it = characters_.find(character_id);
        return it != characters_.end() ? it->second : CharacterPtrType();
    }

    const CharacterMapType& GetAll() const
    {
        return characters_;
    }

    CharacterPtrType Remove(unsigned int character_id)
    {
        auto it = characters_.find(character_id);
        if (it != characters_.end())
        {
            auto ptr = it->second;
            characters_.erase(it);
            return ptr;
        }
        else
        {
            return CharacterPtrType();
        }
    }

    void set_my_character_id(unsigned int character_id)
    {
        my_character_id_ = character_id;
    }

    unsigned int my_character_id() const
    {
        return my_character_id_;
    }

private:
    CharacterMapType characters_;
    unsigned int my_character_id_;
};

CharacterManager::CharacterManager()
    : impl_(new Impl())
{
}

CharacterManager::~CharacterManager()
{
}

void CharacterManager::Add(unsigned int character_id, CharacterPtrType character)
{
    impl_->Add(character_id, character);
}

CharacterManager::CharacterPtrType CharacterManager::Get(unsigned int character_id) const
{
    return impl_->Get(character_id);
}

const CharacterManager::CharacterMapType& CharacterManager::GetAll() const
{
    return impl_->GetAll();
}

CharacterManager::CharacterPtrType CharacterManager::Remove(unsigned int character_id)
{
    return impl_->Remove(character_id);
}

void CharacterManager::set_my_character_id(unsigned int character_id)
{
    impl_->set_my_character_id(character_id);
}

unsigned int CharacterManager::my_character_id() const
{
    return impl_->my_character_id();
}

