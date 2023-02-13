// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <mk@cm4all.com>

#include "Item.hxx"
#include "Stock.hxx"

const char *
CreateStockItem::GetStockName() const noexcept
{
	return stock.GetName();
}

void
CreateStockItem::InvokeCreateError(StockGetHandler &handler,
				   std::exception_ptr ep) noexcept
{
	stock.ItemCreateError(handler, ep);
}

void
CreateStockItem::InvokeCreateAborted() noexcept
{
	stock.ItemCreateAborted();
}

StockItem::~StockItem() noexcept
{
}

const char *
StockItem::GetStockName() const noexcept
{
	return stock.GetName();
}

void
StockItem::Put(bool destroy) noexcept
{
	stock.Put(*this, destroy);
}

void
StockItem::InvokeCreateSuccess(StockGetHandler &handler) noexcept
{
	stock.ItemCreateSuccess(handler, *this);
}

void
StockItem::InvokeCreateError(StockGetHandler &handler,
			     std::exception_ptr ep) noexcept
{
	stock.ItemCreateError(*this, handler, ep);
}

void
StockItem::InvokeCreateAborted() noexcept
{
	stock.ItemCreateAborted(*this);
}

void
StockItem::InvokeIdleDisconnect() noexcept
{
	stock.ItemIdleDisconnect(*this);
}

void
StockItem::InvokeBusyDisconnect() noexcept
{
	stock.ItemBusyDisconnect(*this);
}

void
StockItem::ClearUncleanFlag() noexcept
{
	assert(unclean);
	unclean = false;

	stock.ItemUncleanFlagCleared();
}
