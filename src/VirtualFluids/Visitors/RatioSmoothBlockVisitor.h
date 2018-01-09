#ifndef RatioSmoothBlockVisitor_H
#define RatioSmoothBlockVisitor_H

#include <string>

#include "Block3DVisitor.h"

class Grid3D;
class Block3D;

class RatioSmoothBlockVisitor : public Block3DVisitor
{
public:
   RatioSmoothBlockVisitor(int levelDepth, bool includeNotActiveBlocks = false);

   virtual ~RatioSmoothBlockVisitor() {}

   bool expandsByAdaptation() { return this->expandBlocks; }

   void setExpandByAdaptation(bool expandBlocks);

   int  getLevelRatio() { return this->maxLevelRatio; }
   bool isIterative()   { return true;                }

   void setLevelRatio(int ratio);

   int  getStartLevel();
   int  getStopLevel();

   void setStartLevel(int level);
   void setStopLevel(int level);

   std::string getSpecificDescription();

   void visit(std::shared_ptr<Grid3D> grid, std::shared_ptr<Block3D> block) override;

protected:
   bool lookForExpand(std::shared_ptr<Grid3D> grid, const int& ix1, const int& ix2, const int& ix3, const int& level);
   bool lookForCollapse(std::shared_ptr<Grid3D> grid, const int& ix1, const int& ix2, const int& ix3, const int& level);

private:
   int  maxLevelRatio;
   bool expandBlocks;
   int  levelDepth;
   bool includeNotActiveBlocks;

};

#endif //RatioSmoothBlockVisitor_H
