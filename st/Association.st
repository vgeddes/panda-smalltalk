"
Copyright (c) 2007 Luca Bruno
Copyright (c) 2008 Vincent Geddes

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the 'Software'), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
"

"instance creation"

Association classMethod!
key: aKey value: aValue
	"Create an association aKey -> aValue"
	^ self basicNew key: aKey value: aValue!


"accessing"

Association method!
key
	"Answer the association's key"
	^ key!

Association method!
value
	"Answer the association's value"
	^ value!

Association method!
value: aValue
	"Set the value of the association to aValue"
	value := aValue!


"comparing"

	


"printing"

Association method!
printString
	^ self key printString, '->', self value printString!


"private"

Association method!
key: aKey value: aValue
	"Set the key and the value of the association"
	key := aKey.
	value := aValue!