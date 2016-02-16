//-----------------------------------------------------------------------------
//  This is an implementation of Tom Forsyth's "Linear-Speed Vertex Cache 
//  Optimization" algorithm as described here:
//  http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html
//
//  This code was authored and released into the public domain by
//  Adrian Stone (stone@gameangst.com).
//
//  THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
//  SHALL ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
//  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#pragma once

namespace Graphics
{
	//-----------------------------------------------------------------------------
	//  OptimizeFaces
	//-----------------------------------------------------------------------------
	//  Parameters:
	//      indexList
	//          input index list
	//      indexCount
	//          the number of indices in the list
	//      newIndexList
	//          a pointer to a preallocated buffer the same size as indexList to
	//          hold the optimized index list
	//      lruCacheSize
	//          the size of the simulated post-transform cache (max:64)
	//-----------------------------------------------------------------------------
	template <typename IndexType>
	void OptimizeFaces(const IndexType* indexList, uint32_t indexCount, IndexType* newIndexList, uint16_t lruCacheSize);

	template void OptimizeFaces<uint16_t>(const uint16_t* indexList, uint32_t indexCount, uint16_t* newIndexList, uint16_t lruCacheSize);
	template void OptimizeFaces<uint32_t>(const uint32_t* indexList, uint32_t indexCount, uint32_t* newIndexList, uint16_t lruCacheSize);
}
