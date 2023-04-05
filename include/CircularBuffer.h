#ifndef RING_BUFFER_H
#define RING_BUFFER_H

class CircularBuffer
{
  public:
    CircularBuffer(int capacity);

    void Add(float value);
    void UpdateLast(float value);
    float Get(int index);
    float Last();
    
    
    void Clear();
 
    int GetCount();
    int GetStartIndex();    

    float Max();
    float Min();
    float Average();

    void OutputToSerial();

    float& operator[](int index);

  private:
    float* _values = 0;
    int _startIndex = 0;
    int _count = 0;
    int _capacity = 0;

    int TransformIndex(int index);

};


#endif //RING_BUFFER_H
