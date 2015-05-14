
#ifndef CONSOLE_H_
#define CONSOLE_H_

class Console {
  public:
    Console();
    ~Console();

  private:
#ifdef __WINDOWS__
    unsigned int m_oldCodePage;
    bool m_shouldResetCodePage;
#endif
};

#endif /* CONSOLE_H_ */
