/*
 * GLUS - Modern OpenGL, OpenGL ES and OpenVG Utilities. Copyright (C) 2010 - 2014 Norbert Nopper
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GLUS_FUNCTION_H_
#define GLUS_FUNCTION_H_

#ifdef _MSC_VER
	#define GLUSINLINE static __forceinline
#else
	#define GLUSINLINE static inline
#endif

#ifndef GLUSAPIENTRY
	#ifdef GLAPIENTRY
		#define GLUSAPIENTRY GLAPIENTRY
	#else
		#define GLUSAPIENTRY
	#endif
#endif
#ifndef GLUSAPIENTRYP
#define GLUSAPIENTRYP GLUSAPIENTRY *
#endif
#ifndef GLUSAPI
#define GLUSAPI extern
#endif

#endif /* GLUS_FUNCTION_H_ */
