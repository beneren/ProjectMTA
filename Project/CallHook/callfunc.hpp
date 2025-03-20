#ifndef CALLFUNC_H
#define CALLFUNC_H

namespace CallFunc {
	template<typename T, class ...Args>
	T thiscall(void *object, void *method, Args ... args){
		typedef T(__thiscall *__call)(void *_this, Args ...);
		return (__call(method))(object, args...);
	}
	template<typename T, class ...Args>
	T thiscall(void *object, unsigned int method, Args ... args){
		return thiscall<T>(object, (void*)method, args...);
	}
	template<typename T, class ...Args>
	T thiscall(unsigned int object, void *method, Args ... args){
		return thiscall<T>((void*)object, method, args...);
	}
	template<typename T, class ...Args>
	T thiscall(unsigned int object, unsigned int method, Args ... args){
		return thiscall<T>((void*)object, (void*)method, args...);
	}

	template<typename T, class ...Args>
	T stdcall(void *method, Args ... args){
		typedef T(__stdcall *__call)(Args ...);
		return (__call(method))(args...);
	}
	template<typename T, class ...Args>
	T stdcall(unsigned int method, Args ... args){
		return stdcall<T>((void*)method, args...);
	}

	template<typename T, class ...Args>
	T ccall(void *method, Args ... args){
		typedef T(__cdecl *__call)(Args ...);
		return (__call(method))(args...);
	}
	template<typename T, class ...Args>
	T ccall(unsigned int method, Args ... args){
		return ccall<T>((void*)method, args...);
	}
}

#endif // CALLFUNC_H
