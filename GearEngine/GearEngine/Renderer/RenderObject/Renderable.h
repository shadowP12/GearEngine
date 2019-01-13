#ifndef RENDERABLE_H
#define RENDERABLE_H

class Renderable
{
public:
	Renderable()
	{
		mEnabled = true;
		mDestroyed = false;
	}
	virtual ~Renderable()
	{
	}
	void destroy()
	{
		if (!mDestroyed)
			return;
		mDestroyed = true;
	}
	bool getEnabled()
	{
		return mEnabled;
	}
	void setEnabled(bool enable)
	{
		if (mEnabled != enable)
			mEnabled = enable;
	}
	bool getDestroyed()
	{
		return mDestroyed;
	}
private:
	bool mEnabled;
	bool mDestroyed;
};
#endif
