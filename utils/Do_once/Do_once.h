
/*
    Example of use:

        #include "Do_once.h"

        Do_once send_msg_code;

        void main()
        {
            while (1)
            {
                if (send_msg_code.do_once())  // The code inside this if() will be executed only once.
                {
                    // To rerun this code once, you can call somewhere its reset() method.

                    NRF_LOG_DEBUG("This message will be printed only once");
                    NRF_LOG_FLUSH();
                }
            }
        }
*/

#ifndef _DO_ONCE_H_
#define _DO_ONCE_H_


class Do_once
{
  public:
    bool do_once(void)
    {
        if (flag)
        {
            flag = false;

            return true;
        }

        return false;
    }

    void reset(void)
    {
        flag = true;
    }

  private:
    bool flag = true;
};


#endif // _DO_ONCE_H_
