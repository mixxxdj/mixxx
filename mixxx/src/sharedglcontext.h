#ifndef SHAREDGLCONTEXT_H_
#define SHAREDGLCONTEXT_H_

class QGLContext;

class SharedGLContext 
{
    public:
        static QGLContext* getContext();
    private:
        SharedGLContext() { };
        static QGLContext* s_pSharedGLContext;
};

#endif //SHAREDGLCONTEXT_H_
