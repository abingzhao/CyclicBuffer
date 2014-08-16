/**
 * Author : abing0513
 * 
 */


#ifndef INCLUDED_CYCLICBUFFER_H
#define INCLUDED_CYCLICBUFFER_H
/*******************************************************************************
 *  INCLUDES
 ******************************************************************************/
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <exception>

/*******************************************************************************
 *  Class Delcare
 ******************************************************************************/
namespace cyclicbuffer 
{
    /**
     * @details 循环缓存类，可循环存取任意内置数据类型，目前仅支持内置数据类型
     * 
     * @tparam T 类模板的模板参数，只能是内置数据类型
     */
    template <typename T>
    class CyclicBuffer
    {
    private:
        //! 缓存空间大小(元素的个数)
        size_t mBufferSize;
        //! 缓存的读位置指示
        size_t mReadPtr;
        //! 缓存的写位置指示
        size_t mWritePtr;
        //! 缓存位置，以malloc开辟的一块内存
        T* mBuffer;
    public:
        /**
         * @brief 构造函数
         * @details 构造函数，可接受无参数构造，无参数时候以默认大小(1024)开辟空间
         * 
         * @param bufferSize 缓存空间大小
         */
        CyclicBuffer(size_t bufferSize = 1024);
        ~CyclicBuffer();


        /**
         * @brief 当前缓存中的可读空间
         * @details 返回当前缓存中的可读取空间大小,返回大小为实际空间-1，
         *          目的是方便管理读写指针
         * @return 可读空间大小
         */
        size_t ReadSpace(void) const;


        /**
         * @brief 当前缓存中的可写空间
         * @details 返回当前缓存中的可写入空间大小,返回大小为实际空间-1，
         *          目的是方便管理读写指针
         * @return 可写空间大小
         */
        size_t WriteSpace(void) const;


         /**
          * @brief 向缓存中写入数据
          * @details 向缓存中写入一段数据，输入为要写入数据的指针、以及
          *          本次要写入数据的长度，数据连续存储
          * 
          * @param inPtr 要写入数据的指针
          * @param length 要写入数据的长度
          */
        bool WriteToBuffer(const T * const inPtr, size_t writeLength);


        /**
         * @brief 由缓存的读取数据
         * @details 由缓存中读取一段数据，输入为读取数据的指针、以及
         *          本次要读取数据的长度
         * 
         * @param outPtr 输出数据指针
         * @param length 读取的数据长度
         */ 
        bool ReadFromBuffer(T * const outPtr, size_t readLength) const;


        /**
         * @brief 写入单个数据
         * @details 在当前缓存的结尾处，也就是写指针指向的位置后面写入
         *          一个数据，同时调整读指针
         * 
         * @param ele 要写入的数据
         */
        void PushBack(const T& ele);


        /**
         * @brief 单个数据的读
         * @details 将缓存中第一个(就是读指针指向的位置)元素返回，同时
         *          调整读指针
         * @return 缓存中第一个元素
         */
        T PopFront();


        /**
         * @brief 调整写指针 mWritePtr
         * @details 缓存中数据写指针的调整，与数据读写分开，可通过
         *          定义宏(UPDATE_PTR_MANUALLY)来决定指针调整时刻
         * 
         * @param long 调整长度
         */
        void UpdateReadPtr(signed long size);


        /**
         * @brief 调整读指针 mReadPtr
         * @details 缓存中数据读指针的调整，与数据读写分开，可通过
         *          定义宏(UPDATE_PTR_MANUALLY)来决定指针调整时刻
         * 
         * @param long 调整长度
         */
        void UpdateWritePtr(signed long size);


        /**
         * @brief 重载操作符[]
         * @details 重载操作符[]，以当前读指针位置为开始、写指针位置
         *          为结束，读取缓存中任意位置元素
         * 
         * @param pos 外部传入下标(相对于读指针的偏移)
         */
        T &operator[](size_t pos) const { 
            if(pos > ReadSpace())
                throw std::runtime_error("Out of the data length in buffer!");
            return mBuffer[(mReadPtr+pos)%mBufferSize];
        }


        /**
         * @brief 当前读指针位置
         * @details 读指针位置，缓存起始位置为0
         */
        size_t ReadPtr() const {return mReadPtr;}


        /**
         * @brief 当前写指针位置
         * @details 写指针位置，缓存起始位置为0
         */
        size_t WritePtr() const {return mWritePtr;}
        

        /**
         * @brief 清楚当前缓存数据，即读写指针置0
         */
        void Clear();


        /**
         * @brief 调整缓存空间的大小
         * @details 为满足需要动态调整缓存大小，通过realloc函数实现
         *          数据的处理原则与realloc函数相同
         * 
         * @param newLength 新的缓存空间大小， 
         */
        bool Resize(size_t newLength);
    };
    //==========================================================================
    template <typename T>
    CyclicBuffer<T>::CyclicBuffer(size_t bufferSize)
    :  mBufferSize(bufferSize)
     , mReadPtr(0)
     , mWritePtr(2)
    {
        mBuffer = (T*)malloc(sizeof(T)*mBufferSize);
        if(NULL == mBuffer)
        {
            std::stringstream ss;
            ss  << __func__<< ": the malloc  function failed to allocate "
                << "the requested " << bufferSize*sizeof(T) 
                << "bytes of memory, please check your input bufferSize!";
            throw std::runtime_error(ss.str()); 
        }
        memset(mBuffer,0,sizeof(T)*mBufferSize);
    }
    //==========================================================================
    template <typename T>
    CyclicBuffer<T>::~CyclicBuffer()
    {
        if(NULL != mBuffer)
            free(mBuffer);
    }
    //==========================================================================
    template <typename T>
    size_t CyclicBuffer<T>::ReadSpace(void) const
    {
        // The mWritePtr is ahead of mReadPtr
        if((mReadPtr < mWritePtr) && (mWritePtr - mReadPtr > 1))
        {
            return mWritePtr - mReadPtr - 1;
        }
        // The mReadPtr is ahead of mWritePtr
        if((mReadPtr > mWritePtr) && (mBufferSize + mWritePtr - mReadPtr > 1) )
        {
            return mBufferSize + mWritePtr - mReadPtr - 1;
        }
        return 0;
    }
    //==========================================================================
    template <typename T>
    size_t CyclicBuffer<T>::WriteSpace(void) const
    {
        // The mWritePtr is ahead of mReadPtr
        if( (mReadPtr <= mWritePtr) && (mBufferSize + mReadPtr -mWritePtr > 1))
            return mBufferSize + mReadPtr -mWritePtr - 1;
        // The mReadPtr is ahead of mWritePtr
        if((mReadPtr > mWritePtr) && (mReadPtr - mWritePtr > 1))
            return mReadPtr - mWritePtr - 1;
        return 0;
    }
    //==========================================================================
    template <typename T>
    bool
    CyclicBuffer<T>::WriteToBuffer(const T *inPtr,size_t writeLength)
    {
        if(writeLength > WriteSpace())
        {
            std::stringstream ss;
            ss  << "In " << __func__ << "The requested write length is beyond "
                << "the write buffer size. Please check!";
            std::cout << ss.str() << std::endl;
            return false;
        }
        if (mWritePtr + writeLength <= mBufferSize )
        {
            memcpy(mBuffer+mWritePtr,inPtr,sizeof(T)*writeLength);
        }
        else
        {
            size_t len = mBufferSize - mWritePtr;
            memcpy(mBuffer + mWritePtr, inPtr, sizeof(T) * len);
            memcpy(mBuffer, inPtr + len, sizeof(T)*(writeLength-len));
        }
#ifndef UPDATE_PTR_MANUALLY
        // 没有定义手动调整指针，则需自动调整指针
        UpdateWritePtr(writeLength);
#endif
        return true;
    }
    //==========================================================================
    template <typename T>
    bool CyclicBuffer<T>::ReadFromBuffer(T *outPtr,size_t readLength) const
    {
        if(readLength > ReadSpace())
        {
            std::stringstream ss;
            ss  << "In " << __func__ << "The requested read length is beyond "
                << "the read buffer size. Please check!";
            std::cout << ss.str() << std::endl;
            return false;
        }
        //读取缓存过程
        if (mReadPtr + readLength  <= mBufferSize )
        {
            //当前读指针位置到缓存结束位置的可读空间大于等于本次需要读取的长度，可以一次读取
            memcpy(outPtr,mBuffer+mReadPtr,sizeof(T)*(readLength));
        }
        else 
        {
            //当前读指针位置到缓存结束位置的可读空间小于于本次需要读取的长度，需要分为两次进行读取
            size_t len = mBufferSize - mReadPtr;
            memcpy(outPtr,mBuffer+mReadPtr,sizeof(T)*len);
            memcpy(outPtr+len,mBuffer,sizeof(T)*(readLength-len));
        }

#ifndef UPDATE_PTR_MANUALLY
        // 没有定义手动调整指针，则需自动调整指针
        UpdateReadPtr(readLength);
#endif
        return true;
    }
    //==========================================================================
    template <typename T>
    void CyclicBuffer<T>::PushBack(const T& data)
    {
        if(WriteSpace() == 0)
            throw std::runtime_error("NO space for new data!");
        mBuffer[mWritePtr] = data;
        UpdateWritePtr(1);
    }
    //==========================================================================
    template <typename T>
    T CyclicBuffer<T>::PopFront()
    {
        if(WriteSpace() == 0)
            throw std::runtime_error("NO data in Buffer!");
        T tmpEle = mBuffer[mReadPtr];
        UpdateReadPtr(1);
        return tmpEle;
    }
    //==========================================================================
    template <typename T>
    void  CyclicBuffer<T>::UpdateReadPtr( signed long len)
    {
        mReadPtr = (mReadPtr + len) % mBufferSize;
    }
    //==========================================================================
    template <typename T>
    void 
    CyclicBuffer<T>::UpdateWritePtr(signed long len)
    {
        mWritePtr = (mWritePtr + len) % mBufferSize;
    }
    //==========================================================================
    template <typename T>
    void 
    CyclicBuffer<T>::Clear(void )
    {
        mReadPtr = 0;
        mWritePtr = 0;
    }
    //==========================================================================
    template <typename T>
    bool
    CyclicBuffer<T>::Resize(size_t newLength)
    {
        T * tmpPtr = (T*)realloc(mBuffer,sizeof(T)*newLength);
        if(NULL == tmpPtr)
        {
            std::cout << "The realloc is failed, Check the new length requested"
                      << std::endl;
            return false;
        }
        mBuffer = tmpPtr;
        mBufferSize = newLength;
        return true;
    }
} // namespace cyclicbuffer

#endif /* INCLUDED_CYCLICBUFFER_H */
