
//////////////
////////////////
/////////////////// vector
////////////////
/////////////

template<typename T>
bool vec_contains(const vector<T>& vec, const T& element) {
    return find(vec.begin(), vec.end(), element) != vec.end();
}
