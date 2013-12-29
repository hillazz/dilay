#ifndef DILAY_PARTIAL_ACTION_TRIANGULATE_6_GON
#define DILAY_PARTIAL_ACTION_TRIANGULATE_6_GON

#include <list>
#include "action.hpp"
#include "macro.hpp"

class WingedFace;
class WingedMesh;
class Id;

class PATriangulate6Gon : public Action {
  public: 
    DECLARE_ACTION_BIG5 (PATriangulate6Gon)

  /** `r' = run (m,f,n)` triangulates the 6-gon `f`.
   * Note thate `f.edge ()->firstVertex (f)` must be an even vertex, i.e.
   * it must not be generated during the current subdivision step.
   * The new faces are adjacent to `f`.
   * `f` becomes invalid: use `r` instead.
   * `r` and the new faces's ids are added to `n` if `n` is not `nullptr`.
   */
    WingedFace& run (WingedMesh&, WingedFace&, std::list <Id>* = nullptr);

    void undo ();
    void redo ();
  private:
    class Impl;
    Impl* impl;
};
#endif
