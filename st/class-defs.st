"
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

"Basic"

Class named: 'Object'
	  superclass: 'nil'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'UndefinedObject'
	  superclass: 'Object'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'Behavior'
	  superclass: 'Object'
	  instanceVariableNames: 'format superclass instanceSize methodDictionary instanceVariableNames'
      classVariableNames: ''!

Class named: 'Class'
	  superclass: 'Behavior'
	  instanceVariableNames: 'name classPool'
      classVariableNames: ''!

Class named: 'Metaclass'
	  superclass: 'Behavior'
	  instanceVariableNames: 'instanceClass'
      classVariableNames: ''!


"Collections"

Class named: 'Collection'
	  superclass: 'Object'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'HashedCollection'
	  superclass: 'Collection'
	  instanceVariableNames: 'tally array'
      classVariableNames: ''!

Class named: 'Set'
	  superclass: 'HashedCollection'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'Dictionary'
	  superclass: 'HashedCollection'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'SequenceableCollection'
	  superclass: 'Collection'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'ArrayedCollection'
	  superclass: 'SequenceableCollection'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'Array'
	  superclass: 'ArrayedCollection'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'ByteArray'
	  superclass: 'ArrayedCollection'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'Interval'
	  superclass: 'SequenceableCollection'
	  instanceVariableNames: 'start stop step'
      classVariableNames: ''!

Class named: 'Association'
	  superclass: 'Object'
	  instanceVariableNames: 'key value'
      classVariableNames: ''!


"Streams"

Class named: 'Stream'
	  superclass: 'Object'
	  instanceVariableNames: ''
	  classVariableNames: ''!

Class named: 'PositionableStream'
	  superclass: 'Stream'
	  instanceVariableNames: 'collection position readLimit'
	  classVariableNames: ''!

Class named: 'WriteStream'
	  superclass: 'PositionableStream'
	  instanceVariableNames: 'writeLimit'
	  classVariableNames: ''!


"Execution"

Class named: 'ContextPart'
	  superclass: 'Object'
	  instanceVariableNames: 'sender ip sp'
      classVariableNames: ''!

Class named: 'MethodContext'
	  superclass: 'ContextPart'
	  instanceVariableNames: 'method receiver'
      classVariableNames: ''!

Class named: 'BlockContext'
	  superclass: 'ContextPart'
	  instanceVariableNames: 'initialIP argcount caller home'
      classVariableNames: ''!

Class named: 'CompiledMethod'
	  superclass: 'Object'
	  instanceVariableNames: 'header bytecode literals selector'
      classVariableNames: ''!

Class named: 'Message'
	  superclass: 'Object'
	  instanceVariableNames: 'selector arguments'
      classVariableNames: ''!


"Strings"

Class named: 'String'
	  superclass: 'ArrayedCollection'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'Symbol'
	  superclass: 'String'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'ByteString'
	  superclass: 'String'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'ByteSymbol'
	  superclass: 'String'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'WideString'
	  superclass: 'String'
	  instanceVariableNames: ''
      classVariableNames: ''!


"Numbers"

Class named: 'Magnitude'
	  superclass: 'Object'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'Number'
	  superclass: 'Magnitude'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'Integer'
	  superclass: 'Number'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'SmallInteger'
	  superclass: 'Integer'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'LargeInteger'
	  superclass: 'Integer'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'Fraction'
	  superclass: 'Number'
	  instanceVariableNames: 'numerator denominator'
      classVariableNames: ''!

Class named: 'Float'
	  superclass: 'Number'
	  instanceVariableNames: ''
      classVariableNames: ''!


"Basic data types"

Class named: 'Character'
	  superclass: 'Magnitude'
	  instanceVariableNames: 'value'
      classVariableNames: ''!

Class named: 'Boolean'
	  superclass: 'Object'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'True'
	  superclass: 'Boolean'
	  instanceVariableNames: ''
      classVariableNames: ''!

Class named: 'False'
	  superclass: 'Boolean'
	  instanceVariableNames: ''
      classVariableNames: ''!


Class named: 'Exception'
	  superclass: 'Object'
	  instanceVariableNames: 'messageString'
      classVariableNames: ''!