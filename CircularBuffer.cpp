#include "CircularBuffer.h"

#include <stdlib.h> 
#include <Arduino.h>
    
CircularBuffer::CircularBuffer(int capacity) 
{
  _capacity = capacity;
  _values = (float*)malloc(sizeof(float) * capacity);
}

void CircularBuffer::Add(float value) 
{
  _values[TransformIndex(_count)] = value;

  if(_count < _capacity)
  {
    _count++;
  }
  else
  {
    _startIndex = TransformIndex(1);
  }
}
void CircularBuffer::UpdateLast(float value)
{
  if(_count > 0)
  {
    _values[TransformIndex(_count-1)] = value;
  }
}

float CircularBuffer::Get(int index)
{
  if(_count == 0 || index >= GetCount())
  {
    exit(0);
  }
  else
  {  
    return _values[TransformIndex(index)];
  }
}

float& CircularBuffer::operator[](int index)
{
  if(_count == 0 || index >= GetCount())
  {
    exit(0);
  }

  return _values[TransformIndex(index)];
}

float CircularBuffer::Last()
{
  if(_count == 0)
  {
    return 0;
  }
  else
  {
    return _values[TransformIndex(_count-1)];
  }
}

void CircularBuffer::Clear() 
{
  _count = 0;
}

int CircularBuffer::GetCount() 
{
  return _count;
}    

int CircularBuffer::GetStartIndex() 
{
  return _startIndex;
}    

float CircularBuffer::Max() 
{
  if(_count == 0)
  {
    return 0.0f;
  }
  
  float result = Get(0);

  for(int i = 1; i < _count; ++i)
  {
    if(Get(i) > result)
    {
      result = Get(i);
    }
  }

  return result;
}

float CircularBuffer::Min()
{
  if(_count == 0)
  {
    return 0.0f;
  }
  
  float result = Get(0);

  for(int i = 1; i < _count; ++i)
  {
    if(Get(i) < result)
    {
      result = Get(i);
    }
  }

  return result;
}
float CircularBuffer::Average() 
{
  if(_count == 0)
  {
    return 0.0f;
  }
  
  float result = 0;

  for(int i = 0; i < _count; ++i)
  {
    result += Get(i);
  }

  return result / _count;
}

void CircularBuffer::OutputToSerial()
{
  Serial.println("Raw Data:");
  
  for(int i = 0; i < _capacity; ++i)
  {
    Serial.print(_values[i]);
    Serial.print(",");
  }
  
  Serial.print("Start Index: ");  
  Serial.println(_startIndex);  
  Serial.print("Count: ");  
  Serial.println(_count);  
}

int CircularBuffer::TransformIndex(int index) 
{
  return (_startIndex + index) % _capacity;
}
