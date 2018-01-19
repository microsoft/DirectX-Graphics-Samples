#if defined(_STL110_)

// Razzle builds don't have a definition of std::make_unique<> anywhere to be found.
namespace std
{
    template<class _Ty, class... _Types> inline
        typename enable_if<!is_array<_Ty>::value,
        unique_ptr<_Ty> >::type make_unique(_Types&&... _Args)
    {    // make a unique_ptr
        return (unique_ptr<_Ty>(new _Ty(_STD forward<_Types>(_Args)...)));
    }
}

#endif //_STL110_
