//
//  Logging.h
//  CMIOMinimalSample
//
//  Created by John Boiles  on 4/10/20.
//
//  CMIOMinimalSample is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  CMIOMinimalSample is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with CMIOMinimalSample. If not, see <http://www.gnu.org/licenses/>.

#ifndef Logging_h
#define Logging_h

#define DLog(fmt, ...) NSLog((@"OBSDAL: " fmt), ##__VA_ARGS__)
#define DLogFunc(fmt, ...) NSLog((@"OBSDAL: %s " fmt), __FUNCTION__, ##__VA_ARGS__)

#endif /* Logging_h */
