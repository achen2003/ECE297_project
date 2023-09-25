


// remove duplicates from a vector
// https://www.techiedelight.com/remove-duplicates-vector-cpp/
template <typename Type>
void remove_duplicates(std::vector<Type> &v) {
    std::unordered_set <Type> helper_hash;
    for (Type i : v) {
        helper_hash.insert(i);
    }
    v.assign( helper_hash.begin(), helper_hash.end() );
}