//
// Created by manu343726 on 22/05/15.
//

#ifndef PRACTICA2MAR_UTILS_HPP
#define PRACTICA2MAR_UTILS_HPP

#define AUTO_RETURN(...) -> decltype(__VA_ARGS__) { return __VA_ARGS__; }
#define METHOD_FROM(method, object) template<typename... Args> auto method(Args&&... args) \
                                    AUTO_RETURN(object.method(std::forward<Args>(args)...))

#endif //PRACTICA2MAR_UTILS_HPP
