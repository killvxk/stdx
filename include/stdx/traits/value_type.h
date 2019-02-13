#pragma once
 
 namespace stdx
 {
     template<typename _T>
     struct _ValueType
     {
         using type = _T;
     };
     template<typename _T>
     struct _ValueType<_T&>
     {
         using type = _T;
     };
     template<typename _T>
     struct _ValueType<_T&&>
     {
         using type = _T;
     };
     template<typename _T>
     using value_type = typename _ValueType<_T>::type;
 }
